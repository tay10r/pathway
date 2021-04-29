#include "location.h"

#include <ostream>

void
Location::Dump(std::ostream& os) const
{
  os << first_line << ':' << first_column;

  if ((first_line != last_line) || (first_column != last_column)) {
    os << " to " << last_line << ':' << last_column;
  }
}

std::ostream&
operator<<(std::ostream& os, const Location& l)
{
  os << l.first_line << ':' << l.first_column;
  return os;
}
