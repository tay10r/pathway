#pragma once

#include "common.h"
#include "generator.h"

#include <sstream>

class c_based_generator : public generator
{
public:
  c_based_generator(std::ostream& os_)
    : generator(os_)
  {}

  virtual ~c_based_generator() = default;

protected:
  std::string to_string(Type type) const
  {
    std::ostringstream os;
    os << type.ID();
    return os.str();
  }

  std::ostream& blank() { return this->os << std::endl; }

  std::ostream& comment() { return this->os << "// "; }
};
