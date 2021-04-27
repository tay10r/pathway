#pragma once

#include <ostream>

#include <assert.h>

class Program;

class Generator
{
public:
  Generator(std::ostream& os_)
    : os(os_)
  {}

  virtual ~Generator() = default;

  virtual void Generate(const Program&) = 0;

protected:
  std::ostream& os;

  std::ostream& Indent()
  {
    for (size_t i = 0; i < mIndentLevel; i++)
      this->os << "  ";

    return this->os;
  }

  void IncreaseIndent() { mIndentLevel++; }

  void DecreaseIndent()
  {
    assert(mIndentLevel > 0);

    mIndentLevel--;
  }

private:
  size_t mIndentLevel = 0;
};
