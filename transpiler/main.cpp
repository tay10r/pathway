#include "check.h"
#include "lexer.h"
#include "parse.h"
#include "program.h"
#include "program_consumer.h"
#include "resolve.h"
#include "syntax_error_observer.h"

#include "cpp_generator.h"

#include <fstream>
#include <iostream>
#include <set>
#include <sstream>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

struct FileContext final
{
  std::string path;

  FILE* file;

  FileContext(const char* path_)
    : path(path_)
    , file(fopen(path_, "rb"))
  {}

  FileContext(FileContext&& other)
    : path(std::move(other.path))
    , file(other.file)
  {
    other.file = nullptr;
  }

  ~FileContext()
  {
    if (this->file)
      fclose(this->file);
  }

  bool good() const noexcept { return !!this->file; }
};

class ParserAdapter final
  : public ProgramConsumer
  , public SyntaxErrorObserver
{
public:
  ParserAdapter(const char* program_name_,
                Generator* gen_,
                std::ostream& os_,
                std::ostream& es_ = std::cerr)
    : program_name(program_name_)
    , gen(gen_)
    , os(os_)
    , es(es_)
  {}

  bool BeginFile(const char* path)
  {
    if (!mLexer.PushFile(path)) {
      this->es << this->program_name << ": failed to open '" << path << "' ("
               << strerror(errno) << ')' << std::endl;
      return false;
    }

    mPathStack.emplace_back(path);

    mDependencies.emplace(path);

    return true;
  }

  void EndFile()
  {
    assert(!mPathStack.empty());

    mPathStack.pop_back();

    mLexer.PopFile();
  }

  bool Parse() { return ::Parse(mLexer, *this, *this); }

  bool get_error_flag() const noexcept { return this->error_flag; }

  std::set<std::string> Dependencies() const { return mDependencies; }

  void ConsumeProgram(std::unique_ptr<Program> program) override
  {
    if (this->error_flag)
      return;

    Resolve(*program);

    if (!check(mPathStack.at(0), *program, this->es)) {
      this->error_flag = true;
      return;
    }

    if (mCodeGenEnabled)
      gen->Generate(*program);
  }

  void ObserveSyntaxError(const Location& loc, const char* msg) override
  {
    this->error_flag = true;

    this->es << mPathStack.at(0) << ':';

    this->es << loc << ": " << msg << std::endl;
  }

  void DisableCodeGen() { mCodeGenEnabled = false; }

private:
  std::vector<std::string> mPathStack;
  std::string program_name;
  Lexer mLexer;
  std::unique_ptr<Generator> gen;
  std::ostream& os;
  std::ostream& es;
  std::set<std::string> mDependencies;
  bool error_flag = false;
  bool mCodeGenEnabled = true;
};

const char* options = R"(
options:
  -h, --help            : Print this list of options.
  -l, --language <LANG> : Specify the output language.
  -o, --output <PATH>   : Specify the output path.
)";

void
print_help(const char* argv0)
{
  std::cout << "usage: " << argv0 << " [options] [source_path]" << std::endl;

  std::cout << options;
}

} // namespace

int
main(int argc, char** argv)
{
  std::string source_path;

  std::string lang("c++");

  std::string output_path;

  bool listDependencies = false;

  for (int i = 1; i < argc; i++) {
    if ((strcmp(argv[i], "--output") == 0) || (strcmp(argv[i], "-o") == 0)) {
      if ((i + 1) >= argc) {
        std::cerr << argv[0] << ": '" << argv[i] << "' requires an argument"
                  << std::endl;
        return EXIT_FAILURE;
      }
      output_path = argv[i + 1];
      i++;
    } else if ((strcmp(argv[i], "--language") == 0) ||
               (strcmp(argv[i], "-l") == 0)) {
      if ((i + 1) >= argc) {
        std::cerr << argv[0] << ": '" << argv[i] << "' requires an argument"
                  << std::endl;
        return EXIT_FAILURE;
      }
      lang = argv[i + 1];
      i++;
    } else if (strcmp(argv[i], "--list-dependencies") == 0) {
      listDependencies = true;
    } else if ((strcmp(argv[i], "--help") == 0) ||
               (strcmp(argv[i], "-h") == 0)) {
      print_help(argv[0]);
      return EXIT_FAILURE;
    } else if (argv[i][0] == '-') {
      std::cerr << argv[0] << ": unknown option '" << argv[i] << "'"
                << std::endl;
      return EXIT_FAILURE;
    } else if (!source_path.empty()) {
      std::cerr << argv[0] << ": trailing argument '" << argv[i] << "'"
                << std::endl;
      return EXIT_FAILURE;
    } else {
      source_path = argv[i];
    }
  }

  std::ostringstream output_stream;

  if (lang.empty()) {
    std::cerr << argv[0]
              << ": specify output language with '-l' or '--language'"
              << std::endl;
    return EXIT_FAILURE;
  }

  std::unique_ptr<Generator> gen;

  if (lang == "c++") {
    gen.reset(new CPPGenerator(output_stream));
  } else {
    std::cerr << argv[0] << ": '" << lang << "' is not a supported language."
              << std::endl;
    return EXIT_FAILURE;
  }

  std::string main_path;

  if (!source_path.empty())
    main_path = source_path + "/main.pt";
  else
    main_path = "main.pt";

  ParserAdapter adapter(argv[0], gen.release(), std::cout);

  if (listDependencies)
    adapter.DisableCodeGen();

  if (!adapter.BeginFile(main_path.c_str()))
    return EXIT_FAILURE;

  adapter.Parse();

  adapter.EndFile();

  if (adapter.get_error_flag())
    return EXIT_FAILURE;

  if (listDependencies) {

    auto deps = adapter.Dependencies();

    for (const auto& dep : deps)
      std::cout << dep << std::endl;

    return EXIT_SUCCESS;
  }

  if (output_path.empty()) {
    std::cout << output_stream.str();
    return EXIT_FAILURE;
  }

  std::ofstream output_file(output_path.c_str());

  if (!output_file.good()) {
    std::cerr << argv[0] << ": failed to open '" << output_path << "' ("
              << strerror(errno) << ")" << std::endl;
    return EXIT_FAILURE;
  }

  output_file << output_stream.str();

  return EXIT_SUCCESS;
}
