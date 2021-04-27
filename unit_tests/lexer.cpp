#include <gtest/gtest.h>

#include "lexer.h"

namespace {

std::string
Lex(const std::string& data)
{
  Lexer lexer;

  lexer.PushFile("test.pt", data);

  auto token = lexer.Lex();

  if (!token)
    return "";

  return token->Dump();
}

} // namespace

TEST(Lexer, Identifier)
{
  EXPECT_EQ(Lex("azAZ_09 "), "IDENTIFIER:(azAZ_09):(1:1 to 1:7)");
  EXPECT_EQ(Lex("AZaz_09 "), "IDENTIFIER:(AZaz_09):(1:1 to 1:7)");
  EXPECT_EQ(Lex("_azAZ09 "), "IDENTIFIER:(_azAZ09):(1:1 to 1:7)");

  EXPECT_EQ(Lex("int "), "INT:(1:1 to 1:3)");
  EXPECT_EQ(Lex("float "), "FLOAT:(1:1 to 1:5)");
  EXPECT_EQ(Lex("uniform "), "UNIFORM:(1:1 to 1:7)");
}

TEST(Lexer, IntLiteral)
{
  EXPECT_EQ(Lex("azAZ_09 "), "IDENTIFIER:(azAZ_09):(1:1 to 1:7)");
  EXPECT_EQ(Lex("AZaz_09 "), "IDENTIFIER:(AZaz_09):(1:1 to 1:7)");
  EXPECT_EQ(Lex("_azAZ09 "), "IDENTIFIER:(_azAZ09):(1:1 to 1:7)");

  // TODO test for error handling in int literals
}

TEST(Lexer, FloatLiteral)
{
  EXPECT_EQ(Lex("0. "), "FLOAT_LITERAL:(0.000000e+00):(1:1 to 1:2)");
  EXPECT_EQ(Lex("0.1 "), "FLOAT_LITERAL:(1.000000e-01):(1:1 to 1:3)");
  EXPECT_EQ(Lex("1.2e2 "), "FLOAT_LITERAL:(1.200000e+02):(1:1 to 1:5)");
  EXPECT_EQ(Lex("12.34e+56 "), "FLOAT_LITERAL:(1.234000e+57):(1:1 to 1:9)");
  EXPECT_EQ(Lex("78.91e-23 "), "FLOAT_LITERAL:(7.891000e-22):(1:1 to 1:9)");
  EXPECT_EQ(Lex("45e6 "), "FLOAT_LITERAL:(4.500000e+07):(1:1 to 1:4)");

  // TODO test for error handling in floating point literals
}
