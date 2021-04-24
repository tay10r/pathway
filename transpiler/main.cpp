#include "common.h"
#include "lex.h"
#include "parse.h"

#include "check.h"

#include "cpp_generator.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

namespace {

struct file_context final
{
  std::string path;

  FILE* file;

  file_context(const char* path_)
    : path(path_)
    , file(fopen(path_, "rb"))
  {}

  file_context(file_context&& other)
    : path(std::move(other.path))
    , file(other.file)
  {
    other.file = nullptr;
  }

  ~file_context()
  {
    if (this->file)
      fclose(this->file);
  }

  bool good() const noexcept { return !!this->file; }
};

class parser_adapter final : public parse_observer
{
public:
  parser_adapter(const char* program_name_,
                 generator* gen_,
                 std::ostream& os_,
                 std::ostream& es_ = std::cerr)
    : program_name(program_name_)
    , gen(gen_)
    , os(os_)
    , es(es_)
  {}

  bool begin_file(const char* path)
  {
    file_context ctx(path);

    if (!ctx.good()) {
      this->es << this->program_name << ": failed to open '" << path << "' ("
               << strerror(errno) << ')' << std::endl;
      return false;
    }

    if (this->file_stack.empty())
      yy_switch_to_buffer(yy_create_buffer(ctx.file, YY_BUF_SIZE));
    else
      yypush_buffer_state(yy_create_buffer(ctx.file, YY_BUF_SIZE));

    this->file_stack.emplace_back(std::move(ctx));

    return true;
  }

  void end_file()
  {
    assert(this->file_stack.size() > 0);

    yypop_buffer_state();

    this->file_stack.pop_back();
  }

  bool get_error_flag() const noexcept { return this->error_flag; }

  void on_program(const program& prg) override
  {
    if (this->error_flag)
      return;

    if (!check(this->file_stack[0].path, prg, this->es)) {
      this->error_flag = true;
      return;
    }

    gen->generate(prg);
  }

  void on_syntax_error(const location& loc, const char* msg) override
  {
    this->error_flag = true;

    this->es << current_file_context().path << ':';

    this->es << loc.first_line << ':' << loc.first_column << ": " << msg
             << std::endl;
  }

private:
  const file_context& current_file_context()
  {
    assert(this->file_stack.size() > 0);

    return this->file_stack.back();
  }

  std::string program_name;
  std::vector<file_context> file_stack;
  std::unique_ptr<generator> gen;
  std::ostream& os;
  std::ostream& es;
  bool error_flag = false;
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

  std::string lang;

  std::string output_path;

  for (int i = 1; i < argc; i++) {
    if ((strcmp(argv[i], "--output") == 0) || (strcmp(argv[i], "-o") == 0)) {
      output_path = argv[i + 1];
      i++;
    } else if ((strcmp(argv[i], "--language") == 0) ||
               (strcmp(argv[i], "-l") == 0)) {
      lang = argv[i + 1];
      i++;
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

  std::unique_ptr<generator> gen;

  if (lang == "cxx") {
    gen.reset(new cpp_generator(output_stream));
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

  parser_adapter adapter(argv[0], gen.release(), std::cout);

  if (!adapter.begin_file(main_path.c_str()))
    return EXIT_FAILURE;

  yyparse(adapter);

  adapter.end_file();

  if (adapter.get_error_flag())
    return EXIT_FAILURE;

  std::ofstream output_file(output_path.c_str());

  if (!output_file.good()) {
    std::cerr << argv[0] << ": failed to open '" << output_path << "' ("
              << strerror(errno) << ")" << std::endl;
    return EXIT_FAILURE;
  }

  output_file << output_stream.str();

  return EXIT_SUCCESS;
}
