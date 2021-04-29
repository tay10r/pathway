#pragma once

#include "stmt.h"

#include <sstream>

class Program;

namespace cpp {

class StmtGenerator final : public StmtVisitor
{
public:
  StmtGenerator(const Program& program)
    : mProgram(program)
  {}

  std::string String() const;

  void Visit(const AssignmentStmt&) override;
  void Visit(const CompoundStmt&) override;
  void Visit(const DeclStmt&) override;
  void Visit(const ReturnStmt&) override;

private:
  std::ostream& Indent();

  std::ostringstream mStream;

  size_t mIndentLevel = 0;

  const Program& mProgram;
};

} // namespace cpp
