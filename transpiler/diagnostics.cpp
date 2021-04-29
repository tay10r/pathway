#include "diagnostics.h"

#include <iostream>
#include <sstream>
#include <vector>

#include "abort.h"

#ifdef __unix__
#include <unistd.h>
#endif

std::string_view
ToStringView(Severity severity) noexcept
{
  switch (severity) {
    case Severity::Note:
      return "note";
    case Severity::Warning:
      return "warning";
    case Severity::Error:
      return "error";
  }

  return std::string_view();
}

Severity
GetSeverity(DiagID diagID) noexcept
{
  switch (diagID) {
    case DiagID::SyntaxError:
      return Severity::Error;
  }

  return Severity::Error;
}

namespace {

std::string
AsSpace(size_t lineNumber)
{
  std::ostringstream stream;
  stream << lineNumber;
  auto str = stream.str();
  return std::string(str.size(), ' ');
}

std::string
AsIndent(size_t length, std::string_view data)
{
  std::string indent;

  for (size_t i = 0; (i < length) && (i < data.size()); i++) {
    if (data[i] == '\t')
      indent.push_back('\t');
    else
      indent.push_back(' ');
  }

  return indent;
}

class ConsoleDiagObserverImpl final : public ConsoleDiagObserver
{
public:
  ConsoleDiagObserverImpl(std::ostream& outStream)
    : mStream(outStream)
  {
#ifdef __unix__
    if (&outStream == &std::cerr) {
      mColorEnabled = !!isatty(STDERR_FILENO);
    }
#endif // __unix__
  }

  void BeginFile(const std::string& path, std::string_view data) override
  {
    mFileStack.emplace_back(path, data);
  }

  void EndFile() override
  {
    if (mFileStack.empty()) {
      ABORT("File stack out of balance.");
    }

    mFileStack.pop_back();
  }

  void Observe(const Diag& diag) override
  {
    if (mFileStack.empty()) {
      ABORT("A diagnostic was observed with an empty file stack.");
      return;
    }

    auto [path, data] = mFileStack.back();

    auto loc = diag.GetLocation();

    BeginBoldWhite();

    mStream << path << ':' << diag.GetLocation() << ": ";

    ResetAttribs();

    BeginBoldRed();

    mStream << ToStringView(GetSeverity(diag.ID())) << ':';

    ResetAttribs();

    mStream << std::endl;

    for (size_t line = loc.first_line; line <= loc.last_line; line++) {

      auto lineView = GetLineView(line, data);

      auto clippedLoc = GetClippedLocation(line, data, loc);

      auto indent = AsIndent(clippedLoc.index, lineView);

      mStream << ' ' << line << " | ";

      mStream << lineView.substr(0, clippedLoc.index);

      BeginBoldRed();

      mStream << lineView.substr(clippedLoc.index, clippedLoc.length);

      ResetAttribs();

      mStream << lineView.substr(clippedLoc.index + clippedLoc.length);

      mStream << std::endl;

      mStream << ' ' << AsSpace(line) << " | " << indent;

      BeginBoldRed();

      for (size_t i = 0; i < clippedLoc.length; i++) {
        mStream << '~';
      }

      ResetAttribs();

      mStream << std::endl;

      if (line != loc.first_line)
        continue;

      mStream << ' ' << AsSpace(line) << " | " << indent << diag.Message()
              << std::endl;
    }
  }

  void SetColorEnabled(bool enabled) override { mColorEnabled = enabled; }

private:
  void BeginBoldRed()
  {
    if (mColorEnabled)
      mStream << "\x1b[1;31m";
  }

  void BeginBoldWhite()
  {
    if (mColorEnabled)
      mStream << "\x1b[1;37m";
  }

  void ResetAttribs()
  {
    if (mColorEnabled)
      mStream << "\x1b[0m";
  }

  std::vector<std::pair<std::string, std::string_view>> mFileStack;

  std::ostream& mStream;

  bool mColorEnabled = false;
};

} // namespace

auto
ConsoleDiagObserver::Make(std::ostream& outStream)
  -> std::unique_ptr<ConsoleDiagObserver>
{
  return std::unique_ptr<ConsoleDiagObserver>(
    new ConsoleDiagObserverImpl(outStream));
}

auto
ConsoleDiagObserver::GetLineView(size_t line, std::string_view data) noexcept
  -> std::string_view
{
  size_t lineStart = 0;

  size_t currentLine = 1;

  for (size_t i = 0; (i < data.size()) && (currentLine != line); i++) {

    if (data[i] == '\n') {

      lineStart = i + 1;

      currentLine++;
    }
  }

  if (currentLine != line)
    return std::string_view();

  size_t length = data.size() - lineStart;

  for (size_t i = lineStart; i < data.size(); i++) {
    if ((data[i] == '\r') || (data[i] == '\n')) {
      length = i - lineStart;
      break;
    }
  }

  return data.substr(lineStart, length);
}

auto
ConsoleDiagObserver::GetClippedLocation(size_t line,
                                        std::string_view data,
                                        const Location& loc) noexcept
  -> ClippedLineRange
{
  if ((line < loc.first_line) || (line > loc.last_line))
    return ClippedLineRange{};

  if ((line != loc.first_line) && (line != loc.last_line))
    return ClippedLineRange{ 0, data.size() };

  // TODO : converting a column number to an index requires accounting for
  // UTF-8 continuation bytes (they need to be ignored).

  if (loc.first_line == loc.last_line) {

    size_t index = loc.first_column - 1;

    size_t length = ((loc.last_column - 1) - index) + 1;

    if ((index > data.size()) || ((index + length) > data.size()))
      return ClippedLineRange{};

    return ClippedLineRange{ index, length };
  }

  if (line == loc.first_line) {

    size_t index = loc.first_column - 1;

    if (index > data.size())
      return ClippedLineRange{};

    return ClippedLineRange{ index, data.size() - index };
  }

  if (line == loc.last_line) {

    size_t length = loc.last_column;

    if (length > data.size())
      return ClippedLineRange{};

    return ClippedLineRange{ 0, length };
  }

  return ClippedLineRange{};
}
