#include "abort.h"

#include <iostream>

#include <cstdlib>

[[noreturn]] void
AbortImpl(const char* filename, size_t line, const std::string& message)
{
  std::cerr << "INTERNAL ERROR" << std::endl;
  std::cerr << "==============" << std::endl;
  std::cerr << std::endl;
  std::cerr << "  " << message << std::endl;
  std::cerr << std::endl;
  std::cerr << "  File: " << filename << std::endl;
  std::cerr << "  Line: " << line << std::endl;
  std::cerr << std::endl;
  std::cerr << "Sorry!" << std::endl;
  std::exit(EXIT_FAILURE);
}
