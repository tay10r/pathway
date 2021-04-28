#pragma once

#include "expr.h"
#include "type_environment.h"

#include <sstream>

namespace cpp {

/// For determining if the variable should be a member expression
/// or just a normal, local variable.
enum class VarOrigin
{
  /// Variable is from a declaration statement or parameter.
  Local,
  /// Variable lives in pixel data.
  VaryingGlobal,
  /// Variable lives in frame data.
  UniformGlobal
};

/// For determining if a function call requires passing the uniform data,
/// the varying data, or both.
struct GlobalsUsage final
{
  bool usesUniformGlobals = false;
  bool usesVaryingGlobals = false;
};

template<typename Derived>
class ExprEnvironment
{
public:
  auto GetGlobalsUsage(const FuncCall& funcCall) const
    -> std::optional<GlobalsUsage>
  {
    return GetDerived().GetGlobalsUsageImpl(funcCall);
  }

  auto GetVarOrigin(const VarRef& varRef) const -> std::optional<VarOrigin>
  {
    return GetDerived().GetVarOriginImpl(varRef);
  }

private:
  const Derived& GetDerived() const noexcept
  {
    return static_cast<const Derived&>(*this);
  }
};

template<typename DerivedExprEnv>
class ExprGenerator final : public ExprVisitor
{
public:
  ExprGenerator(const ExprEnvironment<DerivedExprEnv>& exprEnv)
    : mExprEnv(exprEnv)
  {}

  std::string String() const { return mStream.str(); }

  void Visit(const IntLiteral& intLiteral) override
  {
    mStream << "int_type(" << intLiteral.Value() << ')';
  }

  void Visit(const BoolLiteral& boolLiteral) override
  {
    if (boolLiteral.Value())
      mStream << "true";
    else
      mStream << "false";
  }

  void Visit(const FloatLiteral& floatLiteral) override
  {
    mStream << "float_type(" << floatLiteral.Value() << ")";
  }

  void Visit(const BinaryExpr& binaryExpr) override
  {
    binaryExpr.LeftExpr().AcceptVisitor(*this);

    switch (binaryExpr.GetKind()) {
      case BinaryExpr::Kind::Add:
        mStream << " + ";
        break;
      case BinaryExpr::Kind::Sub:
        mStream << " - ";
        break;
      case BinaryExpr::Kind::Mul:
        mStream << " * ";
        break;
      case BinaryExpr::Kind::Div:
        mStream << " / ";
        break;
      case BinaryExpr::Kind::Mod:
        mStream << " % ";
        break;
    }

    binaryExpr.RightExpr().AcceptVisitor(*this);
  }

  void Visit(const FuncCall& funcCall) override
  {
    const auto& args = funcCall.Args();

    mStream << funcCall.Identifier();

    mStream << '(';

    auto globalsUsage = mExprEnv.GetGlobalsUsage(funcCall);
    if (globalsUsage) {
      if (globalsUsage->usesUniformGlobals) {
        if (args.size() > 0)
          mStream << "frame, ";
        else
          mStream << "frame";
      }
    }

    for (size_t i = 0; i < args.size(); i++) {

      args[i]->AcceptVisitor(*this);

      if ((i + 1) < args.size())
        mStream << ", ";
    }

    mStream << ')';
  }

  void Visit(const UnaryExpr& unaryExpr) override
  {
    switch (unaryExpr.GetKind()) {
      case UnaryExpr::Kind::LogicalNot:
        mStream << "!";
        break;
      case UnaryExpr::Kind::BitwiseNot:
        mStream << "~";
        break;
      case UnaryExpr::Kind::Negate:
        mStream << "-";
        break;
    }

    unaryExpr.Recurse(*this);
  }

  void Visit(const GroupExpr&) override {}

  void Visit(const VarRef& varRef) override
  {
    auto origin = mExprEnv.GetVarOrigin(varRef);
    if (!origin) {
      mStream << varRef.Identifier();
      return;
    }

    switch (*origin) {
      case VarOrigin::Local:
        break;
      case VarOrigin::UniformGlobal:
        mStream << "frame." << varRef.Identifier();
        break;
      case VarOrigin::VaryingGlobal:
        mStream << "this->" << varRef.Identifier();
        break;
    }
  }

  void Visit(const type_constructor&) override {}

  void Visit(const MemberExpr&) override {}

private:
  const ExprEnvironment<DerivedExprEnv>& mExprEnv;

  std::ostringstream mStream;
};

} // namespace cpp
