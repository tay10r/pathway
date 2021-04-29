#pragma once

#include "location.h"

#include <iosfwd>
#include <memory>
#include <optional>
#include <string>
#include <string_view>

enum class DiagID
{
  SyntaxError,
  UnresolvedFuncCall,
  UnresolvedVarRef
};

enum class Severity
{
  Note,
  Warning,
  Error
};

std::string_view
ToStringView(Severity severity) noexcept;

Severity
GetSeverity(DiagID diagID) noexcept;

class Location;

class Diag final
{
public:
  Diag(const Location& location, DiagID id, const std::string& msg)
    : mLocation(location)
    , mID(id)
    , mMessage(msg)
  {}

  DiagID ID() const noexcept { return mID; }

  const Location& GetLocation() const noexcept { return mLocation; }

  const std::string& Message() const noexcept { return mMessage; }

private:
  Location mLocation;

  DiagID mID;

  std::string mMessage;
};

class DiagObserver
{
public:
  virtual ~DiagObserver() = default;

  virtual void Observe(const Diag&) = 0;

  virtual void BeginFile(const std::string& path, std::string_view data) = 0;

  virtual void EndFile() = 0;
};

struct ClippedLineRange final
{
  size_t index = 0;
  size_t length = 0;
};

class ConsoleDiagObserver : public DiagObserver
{
public:
  static auto Make(std::ostream& outputStream)
    -> std::unique_ptr<ConsoleDiagObserver>;

  virtual ~ConsoleDiagObserver() = default;

  virtual void SetColorEnabled(bool enabled) = 0;

  // These methods are only exposed for testing.

  /// @brief Gets a view of the entire line in a source file.
  static auto GetLineView(size_t line, std::string_view data) noexcept
    -> std::string_view;

  /// @brief Gets the range of a location, clipped to a single line.
  static auto GetClippedLocation(size_t line,
                                 std::string_view lineData,
                                 const Location&) noexcept -> ClippedLineRange;
};

class DiagErrorFilter final
{
public:
  DiagErrorFilter(DiagObserver& diagObserver)
    : mDiagObserver(diagObserver)
  {}

  bool ErrorFlag() const noexcept { return mErrorFlag; }

  void EmitDiag(const Diag& diag)
  {
    switch (GetSeverity(diag.ID())) {
      case Severity::Note:
      case Severity::Warning:
        break;
      case Severity::Error:
        mErrorFlag = true;
        break;
    }

    mDiagObserver.Observe(diag);
  }

private:
  DiagObserver& mDiagObserver;

  bool mErrorFlag = false;
};
