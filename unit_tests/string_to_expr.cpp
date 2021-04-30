#include "string_to_expr.h"

#include "lexer.h"
#include "module.h"
#include "module_consumer.h"
#include "parse.h"
#include "syntax_error_observer.h"

#include <iostream>
#include <sstream>

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

class ModuleToExprConverter final : public ModuleConsumer
{
public:
  void ConsumeModule(std::unique_ptr<Module> module) override
  {
    if (module->GlobalVars().size() == 1) {
      mExpr = module->GlobalVars()[0]->TakeInitExpr();
    } else {
      std::cerr << "TEST ERROR: Unexpected number of expressions." << std::endl;
    }
  }

  UniqueExprPtr TakeExpr() { return std::move(mExpr); }

private:
  UniqueExprPtr mExpr;
};

} // namespace

UniqueExprPtr
StringToExpr(const std::string& str)
{
  std::ostringstream sourceStream;
  sourceStream << "int expr = " << str << ";" << std::endl;
  auto source = sourceStream.str();

  Lexer lexer;

  lexer.PushFile("test_data.pt", source);

  ModuleToExprConverter converter;

  SyntaxErrorPrinter errorPrinter;

  Parse(lexer, converter, errorPrinter);

  return converter.TakeExpr();
}
