#include "check.h"
#include "diagnostics.h"
#include "lexer.h"
#include "parse.h"
#include "program.h"
#include "program_consumer.h"
#include "resolve.h"
#include "syntax_error_observer.h"

#include "cpp_generator.h"
#include "cpp_generator_v2.h"

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

class Transpiler final
  : public ProgramConsumer
  , public SyntaxErrorObserver
{
public:
  Transpiler(const char* program_name_,
             Generator* gen_,
             std::ostream& os_,
             DiagObserver* diagObserver)
    : program_name(program_name_)
    , gen(gen_)
    , os(os_)
    , mDiagObserver(diagObserver)
  {}

  bool BeginFile(const char* path)
  {
    if (!mLexer.PushFile(path)) {
      std::cerr << this->program_name << ": failed to open '" << path << "' ("
                << strerror(errno) << ')' << std::endl;
      return false;
    }

    mPathStack.emplace_back(path);

    mDependencies.emplace(path);

    mDiagObserver->BeginFile(path, mLexer.GetCurrentFileData());

    return true;
  }

  void EndFile()
  {
    assert(!mPathStack.empty());

    mPathStack.pop_back();

    mLexer.PopFile();

    mDiagObserver->EndFile();
  }

  bool Parse() { return ::Parse(mLexer, *this, *this); }

  bool get_error_flag() const noexcept { return this->error_flag; }

  std::set<std::string> Dependencies() const { return mDependencies; }

  void ConsumeProgram(std::unique_ptr<Program> program) override
  {
    if (this->error_flag)
      return;

    Resolve(*program);

    if (!check(mPathStack.at(0), *program, std::cerr)) {
      this->error_flag = true;
      return;
    }

    if (mCodeGenEnabled)
      gen->Generate(*program);
  }

  void ObserveSyntaxError(const Location& loc, const char* msg) override
  {
    this->error_flag = true;

    Diag diag(loc, DiagID::SyntaxError, msg);

    mDiagObserver->Observe(diag);
  }

  void DisableCodeGen() { mCodeGenEnabled = false; }

private:
  std::vector<std::string> mPathStack;
  std::string program_name;
  Lexer mLexer;
  std::unique_ptr<Generator> gen;
  std::ostream& os;
  std::unique_ptr<DiagObserver> mDiagObserver;
  std::set<std::string> mDependencies;
  bool error_flag = false;
  bool mCodeGenEnabled = true;
};

const char* options = R"(
options:
  -h, --help            : Print this list of options.

  -l, --language <LANG> : Specify the output language.

  -o, --output <PATH>   : Specify the output path.

  --only-if-different   : The output file is only written if it's different from
                          the existing one. Does not have an effect when there
                          is no existing output file.

  --syntax-only         : Only checks syntax, does not generate an output file.
)";

void
PrintHelp(const char* argv0)
{
  std::cout << "usage: " << argv0 << " [options] [source_path]" << std::endl;

  std::cout << options;
}

std::optional<std::string>
ReadWholeFile(const char* path)
{
  std::ifstream file(path);

  if (!file.good())
    return {};

  std::ostringstream stream;

  stream << file.rdbuf();

  return stream.str();
}

} // namespace

int
main(int argc, char** argv)
{
  std::string source_path;

  std::string lang("cxx");

  std::string output_path;

  bool listDependencies = false;

  bool onlyIfDifferent = false;

  bool syntaxOnly = false;

  for (int i = 1; i < argc; i++) {
    if ((strcmp(argv[i], "--output") == 0) || (strcmp(argv[i], "-o") == 0)) {
      if ((i + 1) >= argc) {
        std::cerr << argv[0] << ": '" << argv[i] << "' requires an argument"
                  << std::endl;
        return EXIT_FAILURE;
      }
      output_path = argv[i + 1];
      i++;
    } else if (strcmp(argv[i], "--only-if-different") == 0) {
      onlyIfDifferent = true;
    } else if (strcmp(argv[i], "--syntax-only") == 0) {
      syntaxOnly = true;
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
      PrintHelp(argv[0]);
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

  if (lang == "cxx")
    lang = "cxx_v2";

  if (lang == "cxx_v1") {
    gen.reset(new CPPGenerator(output_stream));
  } else if (lang == "cxx_v2") {
    gen.reset(new cpp::Generator(output_stream));
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

  auto diagObserver = ConsoleDiagObserver::Make(std::cerr);

  Transpiler transpiler(
    argv[0], gen.release(), std::cout, diagObserver.release());

  if (listDependencies)
    transpiler.DisableCodeGen();

  if (!transpiler.BeginFile(main_path.c_str()))
    return EXIT_FAILURE;

  transpiler.Parse();

  transpiler.EndFile();

  if (transpiler.get_error_flag())
    return EXIT_FAILURE;

  if (listDependencies) {

    auto deps = transpiler.Dependencies();

    for (const auto& dep : deps)
      std::cout << dep << std::endl;

    return EXIT_SUCCESS;
  }

  if (syntaxOnly) {
    return EXIT_SUCCESS;
  }

  if (output_path.empty()) {
    std::cout << output_stream.str();
    return EXIT_FAILURE;
  }

  auto output_str = output_stream.str();

  if (onlyIfDifferent) {

    auto existing = ReadWholeFile(output_path.c_str());

    if (existing == output_str)
      return EXIT_SUCCESS;
  }

  std::ofstream output_file(output_path.c_str());

  if (!output_file.good()) {
    std::cerr << argv[0] << ": failed to open '" << output_path << "' ("
              << strerror(errno) << ")" << std::endl;
    return EXIT_FAILURE;
  }

  output_file << output_str;

  return EXIT_SUCCESS;
}
