#include "location.h"

#include <ostream>

std::ostream&
operator<<(std::ostream& os, const Location& l)
{
  os << l.first_line << ':' << l.first_column;

  if ((l.last_line != l.first_line) || (l.last_column != l.first_column))
    os << " to " << l.last_line << ':' << l.last_column;

  return os;
}
