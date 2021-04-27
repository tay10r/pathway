#include "common.h"

#include <ostream>

bool
Var::IsVaryingGlobal() const
{
  return mIsGlobal && mType->IsVaryingOrUnbound();
}

bool
Var::IsUniformGlobal() const
{
  return mIsGlobal && mType->IsUniform();
}

namespace {

class ExprGlobalStateReferenceChecker final : public ExprVisitor
{
public:
  bool ReferencesFrameState() const noexcept { return mReferencesFrameState; }
  bool ReferencesPixelState() const noexcept { return mReferencesPixelState; }

  void Visit(const BoolLiteral&) override {}
  void Visit(const IntLiteral&) override {}
  void Visit(const FloatLiteral&) override {}

  void Visit(const BinaryExpr& binaryExpr) override
  {
    binaryExpr.Recurse(*this);
  }

  void Visit(const FuncCall& funcCall) override
  {
    const auto& funcDecl = funcCall.GetFuncDecl();

    if (funcDecl.ReferencesFrameState())
      mReferencesFrameState = true;

    if (funcDecl.ReferencesPixelState())
      mReferencesPixelState = true;

    funcCall.Recurse(*this);
  }

  void Visit(const unary_expr& unaryExpr) override { unaryExpr.Recurse(*this); }

  void Visit(const GroupExpr& groupExpr) override { groupExpr.Recurse(*this); }

  void Visit(const VarRef& varRef) override
  {
    if (!varRef.HasResolvedVar())
      return;

    const auto& var = varRef.ResolvedVar();

    if (!var.IsGlobal())
      return;

    switch (var.GetVariability()) {
      case Variability::Unbound:
      case Variability::Varying:
        mReferencesPixelState |= true;
        break;
      case Variability::Uniform:
        mReferencesFrameState |= true;
        break;
    }
  }

  void Visit(const type_constructor& typeConstructor) override
  {
    typeConstructor.Recurse(*this);
  }

  void Visit(const MemberExpr& memberExpr) override
  {
    memberExpr.Recurse(*this);
  }

private:
  bool mReferencesFrameState = false;
  bool mReferencesPixelState = false;
};

class StmtGlobalStateReferenceChecker final : public StmtVisitor
{
public:
  bool ReferencesFrameState() const noexcept { return mReferencesFrameState; }

  bool ReferencesPixelState() const noexcept { return mReferencesPixelState; }

  void Visit(const AssignmentStmt& assignmentStmt) override
  {
    CheckExpr(assignmentStmt.LValue());
    CheckExpr(assignmentStmt.RValue());
  }

  void Visit(const DeclStmt& declStmt) override
  {
    if (declStmt.GetVarDecl().init_expr)
      CheckExpr(*declStmt.GetVarDecl().init_expr);
  }

  void Visit(const ReturnStmt& returnStmt) override
  {
    CheckExpr(returnStmt.ReturnValue());
  }

  void Visit(const CompoundStmt& compoundStmt) override
  {
    compoundStmt.Recurse(*this);
  }

private:
  void CheckExpr(const Expr& e)
  {
    ExprGlobalStateReferenceChecker checker;

    e.AcceptVisitor(checker);

    mReferencesFrameState |= checker.ReferencesFrameState();
    mReferencesPixelState |= checker.ReferencesPixelState();
  }

  bool mReferencesFrameState = false;
  bool mReferencesPixelState = false;
};

} // namespace

bool
Func::ReferencesFrameState() const
{
  StmtGlobalStateReferenceChecker checker;

  this->mBody->AcceptVisitor(checker);

  return checker.ReferencesFrameState();
}

bool
Func::ReferencesPixelState() const
{
  StmtGlobalStateReferenceChecker checker;

  this->mBody->AcceptVisitor(checker);

  return checker.ReferencesPixelState();
}

bool
Func::ReferencesGlobalState() const
{
  return ReferencesFrameState() || ReferencesPixelState();
}

bool
Func::IsEntryPoint() const
{
  return IsPixelSampler() || IsPixelEncoder();
}

bool
Func::IsPixelSampler() const
{
  return this->mName.Identifier() == "SamplePixel";
}

bool
Func::IsPixelEncoder() const
{
  return this->mName.Identifier() == "EncodePixel";
}

void
Program::AppendGlobalVar(Var* globalVar)
{
  globalVar->MarkAsGlobal();

  mGlobalVars.emplace_back(globalVar);

  switch (globalVar->GetVariability()) {
    case Variability::Uniform:
      mUniformGlobalVars.emplace_back(globalVar);
      break;
    case Variability::Varying:
    case Variability::Unbound:
      mVaryingGlobalVars.emplace_back(globalVar);
      break;
  }
}
