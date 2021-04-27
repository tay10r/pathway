#pragma once

#include <sstream>

#include <stddef.h>

[[noreturn]] void
AbortImpl(const char* filename, size_t line, const std::string& message);

template<typename... Args>
[[noreturn]] void
Abort(const char* filename, size_t line, Args... args)
{
  std::ostringstream stream;

  (stream << ... << args);

  AbortImpl(filename, line, stream.str());
}

#define ABORT(...) Abort(__FILE__, __LINE__, __VA_ARGS__)
