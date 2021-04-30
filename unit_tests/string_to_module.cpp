#include "string_to_module.h"

#include "lexer.h"
#include "module.h"
#include "module_consumer.h"
#include "parse.h"
#include "syntax_error_observer.h"

#include <iostream>

namespace {

class SyntaxErrorPrinter final : public SyntaxErrorObserver
{
public:
  void ObserveSyntaxError(const Location& location,
                          const char* message) override
  {
    std::cerr << "TEST ERROR: " << location << message;
  }
};

class ModuleSaver final : public ModuleConsumer
{
public:
  void ConsumeModule(std::unique_ptr<Module> module) override
  {
    mModule = std::move(module);
  }

  std::unique_ptr<Module> TakeModule() { return std::move(mModule); }

private:
  std::unique_ptr<Module> mModule;
};

} // namespace

auto
StringToModule(const std::string& source) -> std::unique_ptr<Module>
{
  Lexer lexer;

  lexer.PushFile("test_data.pt", source);

  ModuleSaver saver;

  SyntaxErrorPrinter errorPrinter;

  Parse(lexer, saver, errorPrinter);

  return saver.TakeModule();
}
