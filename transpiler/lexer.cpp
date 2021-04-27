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
    case YYEMPTY:
      stream << "YYEMPTY";
      break;
    case END:
      stream << "END";
      break;
    case YYerror:
      stream << "YYerror";
      break;
    case YYUNDEF:
      stream << "YYUNDEF";
      break;
    case INT_LITERAL:
      stream << "INT_LITERAL";
      break;
    case FLOAT_LITERAL:
      stream << "FLOAT_LITERAL";
      break;
    case BOOL_LITERAL:
      stream << "BOOL_LITERAL";
      break;
    case IDENTIFIER:
      stream << "IDENTIFIER";
      break;
    case RETURN:
      stream << "RETURN";
      break;
    case BREAK:
      stream << "BREAK";
      break;
    case CONTINUE:
      stream << "CONTINUE";
      break;
    case IF:
      stream << "IF";
      break;
    case ELSE:
      stream << "ELSE";
      break;
    case FOR:
      stream << "FOR";
      break;
    case WHILE:
      stream << "WHILE";
      break;
    case INVALID_CHAR:
      stream << "INVALID_CHAR";
      break;
    case VOID:
      stream << "VOID";
      break;
    case BOOL:
      stream << "BOOL";
      break;
    case INT:
      stream << "INT";
      break;
    case FLOAT:
      stream << "FLOAT";
      break;
    case VEC2:
      stream << "VEC2";
      break;
    case VEC3:
      stream << "VEC3";
      break;
    case VEC4:
      stream << "VEC4";
      break;
    case VEC2I:
      stream << "VEC2I";
      break;
    case VEC3I:
      stream << "VEC3I";
      break;
    case VEC4I:
      stream << "VEC4I";
      break;
    case MAT2:
      stream << "MAT2";
      break;
    case MAT3:
      stream << "MAT3";
      break;
    case MAT4:
      stream << "MAT4";
      break;
    case UNIFORM:
      stream << "UNIFORM";
      break;
    case VARYING:
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

  stream << ":(" << mLocation << ')';

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

  idMap["uniform"] = UNIFORM;
  idMap["varying"] = VARYING;

  idMap["void"] = VOID;
  idMap["bool"] = BOOL;
  idMap["int"] = INT;
  idMap["float"] = FLOAT;

  idMap["vec2"] = VEC2;
  idMap["vec3"] = VEC3;
  idMap["vec4"] = VEC4;

  idMap["vec2i"] = VEC2I;
  idMap["vec3i"] = VEC3I;
  idMap["vec4i"] = VEC4I;

  idMap["mat2"] = MAT2;
  idMap["mat3"] = MAT3;
  idMap["mat4"] = MAT4;

  idMap["break"] = BREAK;
  idMap["continue"] = CONTINUE;
  idMap["return"] = RETURN;

  idMap["if"] = IF;
  idMap["else"] = ELSE;
  idMap["for"] = FOR;
  idMap["while"] = WHILE;
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

  return Token(IDENTIFIER, identifier, location);
}

Token
Lexer::ProduceInteger(size_t length)
{
  auto& context = CurrentContext();

  auto view = context.MakeStringView(length);

  std::string dataCopy(view);

  auto value = strtoul(dataCopy.c_str(), nullptr, 10);

  auto location = context.AdvanceAndProduceLocation(length);

  return Token(INT_LITERAL, uint64_t(value), location);
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

  return Token(FLOAT_LITERAL, value, location);
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
