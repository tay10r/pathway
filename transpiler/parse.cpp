#include "parse.h"

#include "generated/parse.h"

bool
Parse(Lexer& lexer,
      ProgramConsumer& programConsumer,
      SyntaxErrorObserver& errorObserver)
{
  return yyparse(lexer, programConsumer, errorObserver) == 0;
}
