#pragma once

#include "cpp_extensions.h"
#include "expr.h"
#include "type_environment.h"

#include <sstream>

namespace cpp {

class ExprGenerator final : public ExprVisitor
{
public:
  ExprGenerator(std::shared_ptr<cpp::DataMode> dataMode)
    : mDataMode(dataMode)
  {}

  std::string String() const { return mStream.str(); }

  void Visit(const IntLiteral& intLiteral) override
  {
    mStream << intLiteral.Value();

    if (mDataMode->ScalarBits() == 64)
      mStream << "ll";
    else
      mStream << "l";
  }

  void Visit(const BoolLiteral&) override {}

  void Visit(const FloatLiteral&) override {}

  void Visit(const BinaryExpr& binaryExpr) override
  {
    //
    (void)binaryExpr;
  }

  void Visit(const unary_expr&) override {}

  void Visit(const GroupExpr&) override {}

  void Visit(const VarRef&) override {}

  void Visit(const type_constructor&) override {}

  void Visit(const MemberExpr&) override {}

private:
  std::shared_ptr<DataMode> mDataMode;

  std::ostringstream mStream;
};

} // namespace cpp
