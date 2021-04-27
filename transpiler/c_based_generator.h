#pragma once

#include "common.h"
#include "generator.h"

#include <sstream>

class CBasedGenerator : public Generator
{
public:
  CBasedGenerator(std::ostream& os_)
    : Generator(os_)
  {}

  virtual ~CBasedGenerator() = default;

protected:
  std::string ToString(Type type) const
  {
    std::ostringstream os;
    os << type.ID();
    return os.str();
  }

  std::ostream& Blank() { return this->os << std::endl; }

  std::ostream& Comment() { return this->os << "// "; }
};
