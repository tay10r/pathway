%{
#include "common.h"
#include "parse.h"

#include <assert.h>
#include <stdlib.h>

namespace {

void update_loc(location* loc, const char* str, int len)
{
  assert(len >= 0);

  loc->first_line = loc->last_line;
  loc->first_column = loc->last_column;

  for (int i = 0; i < len; i++) {
    if (str[i] == '\n') {
      loc->last_line++;
      loc->last_column = 1;
    } else {
      loc->last_column++;
    }
  }
}

} // namespace

#define YY_USER_ACTION update_loc(loc, yytext, yyleng);

%}

%option yylineno
%option nounput
%option noyywrap
%option nounistd
%option never-interactive

%%

"bool" {
  val->asTypeID = TypeID::Bool;
  return TYPE;
}

"int" {
  val->asTypeID = TypeID::Int;
  return TYPE;
}

"float" {
  val->asTypeID = TypeID::Float;
  return TYPE;
}

"vec2" {
  val->asTypeID = TypeID::Vec2;
  return TYPE;
}

"vec3" {
  val->asTypeID = TypeID::Vec3;
  return TYPE;
}

"vec4" {
  val->asTypeID = TypeID::Vec4;
  return TYPE;
}

"vec2i" {
  val->asTypeID = TypeID::Vec2i;
  return TYPE;
}

"vec3i" {
  val->asTypeID = TypeID::Vec3i;
  return TYPE;
}

"vec4i" {
  val->asTypeID = TypeID::Vec4i;
  return TYPE;
}

"mat2" {
  val->asTypeID = TypeID::Mat2;
  return TYPE;
}

"mat3" {
  val->asTypeID = TypeID::Mat3;
  return TYPE;
}

"mat4" {
  val->asTypeID = TypeID::Mat4;
  return TYPE;
}

"return" { return RETURN; }

[a-zA-Z_][a-zA-Z0-9_]* {
  val->as_string = new std::string(yytext, yyleng);
  return IDENTIFIER;
}

[0-9][0-9]*(\.[0-9]*) {
  val->as_float = strtod(yytext, nullptr);
  return FLOAT_LITERAL;
}

[0-9][0-9]* {
  val->as_int = strtoul(yytext, nullptr, 10);
  return INT_LITERAL;
}

[ \t\r] { }

\n { }

[{}();:,+\-*/=.] { return yytext[0]; }

<<EOF>> { return END; }

. {
  // TODO : try to match a full UTF-8 sequence here.
  val->invalid_char = yytext[0];
  return INVALID_CHAR;
}
