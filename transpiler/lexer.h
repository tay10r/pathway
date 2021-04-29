#pragma once

#include "location.h"

#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

using TokenData = std::variant<double, uint64_t, std::string_view>;

class Token final
{
public:
  Token() = delete;

  std::string Dump() const;

  int Kind() const noexcept { return mKind; }

  Location GetLocation() const noexcept { return mLocation; }

  uint64_t AsInt() const { return std::get<uint64_t>(mData.value()); }

  double AsDouble() const { return std::get<double>(mData.value()); }

  std::string_view AsStringView() const
  {
    return std::get<std::string_view>(mData.value());
  }

private:
  friend class Lexer;

  Token(int kind, TokenData data, Location location)
    : mKind(kind)
    , mData(std::move(data))
    , mLocation(location)
  {}

  Token(int kind, Location location)
    : mKind(kind)
    , mLocation(location)
  {}

  int mKind;

  std::optional<TokenData> mData;

  Location mLocation;
};

class Lexer final
{
public:
  Lexer();

  std::optional<Token> Lex();

  bool PushFile(const char* path);

  void PushFile(const char* path, std::string data);

  void PopFile();

  std::string_view GetCurrentFileData() const noexcept;

private:
  Token ProduceIdentifier(size_t length);

  Token ProduceInteger(size_t length);

  Token CompleteFloatAfterDot(size_t startingOffset);

  Token CompleteFloatAfterE(size_t startingOffset);

  Token ProduceFloat(size_t length);

  class FileContext;

  FileContext& CurrentContext();

  class FileContext final
  {
  public:
    FileContext(const std::string& path, std::string&& data)
      : mPath(path)
      , mData(std::move(data))
    {}

    bool AtEnd() const noexcept;

    auto MakeStringView(size_t length) const -> std::string_view;

    char Peek(size_t relOffset) const noexcept;

    void Advance(size_t charCount) noexcept;

    Location AdvanceAndProduceLocation(size_t charCount) noexcept;

    void SkipUseless() noexcept;

    bool IsOutOfBounds(size_t relOffset) const noexcept
    {
      return (relOffset + mCurrentIndex) >= mData.size();
    }

    size_t Remaining() const noexcept
    {
      return (mCurrentIndex < mData.size()) ? mData.size() - mCurrentIndex : 0;
    }

    std::string_view Data() const noexcept { return mData; }

  private:
    std::string mPath;
    std::string mData;
    size_t mCurrentIndex = 0;
    size_t mCurrentLine = 1;
    size_t mCurrentColumn = 1;
  };

  std::vector<FileContext> mFileContextStack;

  std::map<std::string_view, int> mIdentifierOverrideMap;
};
