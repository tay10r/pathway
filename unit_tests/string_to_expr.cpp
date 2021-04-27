#include "string_to_expr.h"

#include "common.h"
#include "lexer.h"
#include "parse.h"
#include "program_consumer.h"
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

class ProgramToExprConverter final : public ProgramConsumer
{
public:
  void ConsumeProgram(std::unique_ptr<Program> program) override
  {
    if (program->GlobalVars().size() == 1) {
      mExpr = program->GlobalVars()[0]->TakeInitExpr();
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

  ProgramToExprConverter converter;

  SyntaxErrorPrinter errorPrinter;

  Parse(lexer, converter, errorPrinter);

  return converter.TakeExpr();
}
