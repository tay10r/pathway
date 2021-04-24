#pragma once

#include <ostream>

#include <assert.h>

struct program;

class generator
{
public:
  generator(std::ostream& os_)
    : os(os_)
  {}

  virtual ~generator() = default;

  virtual void generate(const program&) = 0;

protected:
  std::ostream& os;

  std::ostream& indent()
  {
    for (size_t i = 0; i < this->indent_level; i++)
      this->os << "  ";

    return this->os;
  }

  void increase_indent() { this->indent_level++; }

  void decrease_indent()
  {
    assert(this->indent_level > 0);

    this->indent_level--;
  }

private:
  size_t indent_level = 0;
};
