#include "lexer.h"

#include "generated/parse.h"

#include <fstream>
#include <iomanip>
#include <sstream>

std::string
Token::Dump() const
{
  std::ostringstream stream;

  switch (static_cast<yytokentype>(mKind)) {
    case TOK_YYEMPTY:
      stream << "YYEMPTY";
      break;
    case TOK_END:
      stream << "END";
      break;
    case TOK_YYerror:
      stream << "YYerror";
      break;
    case TOK_YYUNDEF:
      stream << "YYUNDEF";
      break;
    case TOK_MODULE:
      stream << "MODULE";
      break;
    case TOK_IMPORT:
      stream << "IMPORT";
      break;
    case TOK_EXPORT:
      stream << "EXPORT";
      break;
    case TOK_INT_LITERAL:
      stream << "INT_LITERAL";
      break;
    case TOK_FLOAT_LITERAL:
      stream << "FLOAT_LITERAL";
      break;
    case TOK_PI:
      stream << "PI";
      break;
    case TOK_INFINITY:
      stream << "INFINITY";
      break;
    case TOK_TRUE:
      stream << "TRUE";
      break;
    case TOK_FALSE:
      stream << "FALSE";
      break;
    case TOK_IDENTIFIER:
      stream << "IDENTIFIER";
      break;
    case TOK_RETURN:
      stream << "RETURN";
      break;
    case TOK_BREAK:
      stream << "BREAK";
      break;
    case TOK_CONTINUE:
      stream << "CONTINUE";
      break;
    case TOK_IF:
      stream << "IF";
      break;
    case TOK_ELSE:
      stream << "ELSE";
      break;
    case TOK_FOR:
      stream << "FOR";
      break;
    case TOK_WHILE:
      stream << "WHILE";
      break;
    case TOK_INVALID_CHAR:
      stream << "INVALID_CHAR";
      break;
    case TOK_VOID:
      stream << "VOID";
      break;
    case TOK_BOOL:
      stream << "BOOL";
      break;
    case TOK_INT:
      stream << "INT";
      break;
    case TOK_FLOAT:
      stream << "FLOAT";
      break;
    case TOK_VEC2:
      stream << "VEC2";
      break;
    case TOK_VEC3:
      stream << "VEC3";
      break;
    case TOK_VEC4:
      stream << "VEC4";
      break;
    case TOK_VEC2I:
      stream << "VEC2I";
      break;
    case TOK_VEC3I:
      stream << "VEC3I";
      break;
    case TOK_VEC4I:
      stream << "VEC4I";
      break;
    case TOK_MAT2:
      stream << "MAT2";
      break;
    case TOK_MAT3:
      stream << "MAT3";
      break;
    case TOK_MAT4:
      stream << "MAT4";
      break;
    case TOK_UNIFORM:
      stream << "UNIFORM";
      break;
    case TOK_VARYING:
      stream << "VARYING";
      break;
  }

  if (mData.has_value()) {

    stream << ":(";

    // TODO : just use visitor method here.

    if (std::holds_alternative<uint64_t>(mData.value()))
      stream << std::get<uint64_t>(mData.value());
    else if (std::holds_alternative<double>(mData.value()))
      stream << std::scientific << std::get<double>(mData.value());
    else if (std::holds_alternative<std::string_view>(mData.value()))
      stream << std::get<std::string_view>(mData.value());

    stream << ')';
  }

  stream << ":(";

  mLocation.Dump(stream);

  stream << ')';

  return stream.str();
}

namespace {

bool
InRange(char c, char lo, char hi) noexcept
{
  return (c >= lo) && (c <= hi);
}

bool
IsNonDigit(char c) noexcept
{
  return InRange(c, 'a', 'z') || InRange(c, 'A', 'Z') || (c == '_');
}

bool
IsDigit(char c) noexcept
{
  return InRange(c, '0', '9');
}

char
ToLower(char c) noexcept
{
  return ((c >= 'A') && (c <= 'Z')) ? c + 32 : c;
}

} // namespace

Lexer::Lexer()
{
  auto& idMap = mIdentifierOverrideMap;

  idMap["true"] = TOK_TRUE;
  idMap["false"] = TOK_FALSE;
  idMap["pi"] = TOK_PI;
  idMap["infinity"];

  idMap["module"] = TOK_MODULE;
  idMap["import"] = TOK_IMPORT;
  idMap["export"] = TOK_EXPORT;

  idMap["uniform"] = TOK_UNIFORM;
  idMap["varying"] = TOK_VARYING;

  idMap["void"] = TOK_VOID;
  idMap["bool"] = TOK_BOOL;
  idMap["int"] = TOK_INT;
  idMap["float"] = TOK_FLOAT;

  idMap["vec2"] = TOK_VEC2;
  idMap["vec3"] = TOK_VEC3;
  idMap["vec4"] = TOK_VEC4;

  idMap["vec2i"] = TOK_VEC2I;
  idMap["vec3i"] = TOK_VEC3I;
  idMap["vec4i"] = TOK_VEC4I;

  idMap["mat2"] = TOK_MAT2;
  idMap["mat3"] = TOK_MAT3;
  idMap["mat4"] = TOK_MAT4;

  idMap["break"] = TOK_BREAK;
  idMap["continue"] = TOK_CONTINUE;
  idMap["return"] = TOK_RETURN;

  idMap["if"] = TOK_IF;
  idMap["else"] = TOK_ELSE;
  idMap["for"] = TOK_FOR;
  idMap["while"] = TOK_WHILE;
}

std::optional<Token>
Lexer::Lex()
{
  if (mFileContextStack.empty())
    return {};

  auto& currentContext = CurrentContext();

  currentContext.SkipUseless();

  if (currentContext.AtEnd()) {
    // TODO : What if there's more files in the context stack?
    return {};
  }

  auto first = currentContext.Peek(0);

  if (IsNonDigit(first)) {

    size_t length = 1;

    while (!currentContext.IsOutOfBounds(length)) {

      auto c = currentContext.Peek(length);

      if (!IsNonDigit(c) && !IsDigit(c))
        break;
      else
        length++;
    }

    return ProduceIdentifier(length);
  }

  if (IsDigit(first)) {

    size_t length = 1;

    while (!currentContext.IsOutOfBounds(length)) {

      auto c = currentContext.Peek(length);

      if (IsDigit(c)) {
        length++;
      } else if (c == '.') {
        length++;
        return CompleteFloatAfterDot(length);
      } else if (ToLower(c) == 'e') {
        length++;
        return CompleteFloatAfterE(length);
      } else {
        break;
      }
    }

    return ProduceInteger(length);
  }

  return Token(first, currentContext.AdvanceAndProduceLocation(1));
}

std::string_view
Lexer::GetCurrentFileData() const noexcept
{
  if (mFileContextStack.empty())
    return std::string_view();

  return mFileContextStack.back().Data();
}

Token
Lexer::ProduceIdentifier(size_t length)
{
  auto& currentContext = CurrentContext();

  auto identifier = currentContext.MakeStringView(length);

  auto location = currentContext.AdvanceAndProduceLocation(length);

  auto it = mIdentifierOverrideMap.find(identifier);

  if (it != mIdentifierOverrideMap.end())
    return Token(it->second, location);

  // definitely an identifier at this point.

  return Token(TOK_IDENTIFIER, identifier, location);
}

Token
Lexer::ProduceInteger(size_t length)
{
  auto& context = CurrentContext();

  auto view = context.MakeStringView(length);

  std::string dataCopy(view);

  auto value = strtoul(dataCopy.c_str(), nullptr, 10);

  auto location = context.AdvanceAndProduceLocation(length);

  return Token(TOK_INT_LITERAL, uint64_t(value), location);
}

Token
Lexer::CompleteFloatAfterDot(size_t length)
{
  auto& context = CurrentContext();

  while (!context.IsOutOfBounds(length)) {

    auto c = context.Peek(length);

    if (ToLower(c) == 'e') {
      return CompleteFloatAfterE(length + 1);
    }

    if (IsDigit(c)) {
      length++;
      continue;
    }

    break;
  }

  return ProduceFloat(length);
}

Token
Lexer::CompleteFloatAfterE(size_t length)
{
  auto& context = CurrentContext();

  if ((context.Peek(length == '+') || (context.Peek(length == '-')))) {
    length++;
  }

  while (!context.IsOutOfBounds(length)) {
    if (!IsDigit(context.Peek(length)))
      break;
    else
      length++;
  }

  return ProduceFloat(length);
}

Token
Lexer::ProduceFloat(size_t length)
{
  auto& context = CurrentContext();

  auto view = context.MakeStringView(length);

  std::string dataCopy(view);

  auto value = strtod(dataCopy.c_str(), nullptr);

  auto location = context.AdvanceAndProduceLocation(length);

  return Token(TOK_FLOAT_LITERAL, value, location);
}

Location
Lexer::FileContext::AdvanceAndProduceLocation(size_t charCount) noexcept
{
  Location location;

  location.first_line = mCurrentLine;

  location.first_column = mCurrentColumn;

  Advance(charCount - 1);

  location.last_line = mCurrentLine;

  location.last_column = mCurrentColumn;

  Advance(1);

  return location;
}

bool
Lexer::PushFile(const char* path)
{
  std::ifstream file(path);

  if (!file.good())
    return false;

  std::ostringstream fileStream;

  fileStream << file.rdbuf();

  PushFile(path, fileStream.str());

  return true;
}

void
Lexer::PushFile(const char* path, std::string data)
{
  FileContext fileContext(path, std::move(data));

  mFileContextStack.emplace_back(std::move(fileContext));
}

void
Lexer::PopFile()
{
  assert(!mFileContextStack.empty());

  mFileContextStack.pop_back();
}

Lexer::FileContext&
Lexer::CurrentContext()
{
  assert(!mFileContextStack.empty());

  return mFileContextStack.back();
}

bool
Lexer::FileContext::AtEnd() const noexcept
{
  return mCurrentIndex >= mData.size();
}

std::string_view
Lexer::FileContext::MakeStringView(size_t length) const
{
  const auto* startPtr = mData.data() + mCurrentIndex;

  length = (length < Remaining()) ? length : Remaining();

  return std::string_view(startPtr, length);
}

char
Lexer::FileContext::Peek(size_t relOffset) const noexcept
{
  size_t absOffset = mCurrentIndex + relOffset;

  return (absOffset < mData.size()) ? mData[absOffset] : 0;
}

void
Lexer::FileContext::Advance(size_t count) noexcept
{
  for (size_t i = 0; i < count; i++) {

    auto c = Peek(i);

    if (c == '\n') {
      mCurrentLine++;
      mCurrentColumn = 1;
    } else {
      mCurrentColumn++;
    }
  }

  mCurrentIndex += count;
}

void
Lexer::FileContext::SkipUseless() noexcept
{
  while (!AtEnd()) {

    auto c = Peek(0);

    if ((c == ' ') || (c == '\t') || (c == '\r') || (c == '\n')) {
      Advance(1);
      continue;
    }

    break;
  }
}
