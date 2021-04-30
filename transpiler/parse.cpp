#include "parse.h"

#include "generated/parse.h"

bool
Parse(Lexer& lexer,
      ModuleConsumer& moduleConsumer,
      SyntaxErrorObserver& errorObserver)
{
  return yyparse(lexer, moduleConsumer, errorObserver) == 0;
}
