#pragma once

#include <stddef.h>

#include <iosfwd>

struct Location final
{
  size_t first_line = 1;

  size_t first_column = 1;

  size_t last_line = 1;

  size_t last_column = 1;
};

std::ostream&
operator<<(std::ostream&, const Location& location);
