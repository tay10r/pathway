#include "resolution_check_pass.h"

#include "decl.h"
#include "diagnostics.h"
#include "expr.h"

namespace {

class ExprResolutionChecker final : public ExprVisitor
{
public:
  ExprResolutionChecker(DiagErrorFilter& errorFilter)
    : mErrorFilter(errorFilter)
  {}

  void Visit(const FuncCall& funcCall) override
  {
    if (!funcCall.Resolved()) {

      auto nameLoc = funcCall.GetNameLocation();

      Diag diag(
        nameLoc, DiagID::UnresolvedFuncCall, "unable to find this function");

      mErrorFilter.EmitDiag(diag);
    }

    funcCall.Recurse(*this);
  }

  void Visit(const IntLiteral&) override {}

  void Visit(const BoolLiteral&) override {}

  void Visit(const FloatLiteral&) override {}

  void Visit(const BinaryExpr& binaryExpr) override
  {
    binaryExpr.Recurse(*this);
  }

  void Visit(const UnaryExpr& unaryExpr) override { unaryExpr.Recurse(*this); }

  void Visit(const GroupExpr& groupExpr) override { groupExpr.Recurse(*this); }

  void Visit(const VarRef& varRef) override { (void)varRef; }

  void Visit(const TypeConstructor& typeConstructor) override
  {
    typeConstructor.Recurse(*this);
  }

  void Visit(const MemberExpr& memberExpr) override
  {
    memberExpr.Recurse(*this);

    // TODO : Check member
  }

private:
  DiagErrorFilter& mErrorFilter;
};

class StmtResolutionChecker final : public StmtVisitor
{
public:
  StmtResolutionChecker(DiagErrorFilter& errorFilter)
    : mExprChecker(errorFilter)
  {}

  void Visit(const AssignmentStmt& assignmentStmt) override
  {
    assignmentStmt.LValue().AcceptVisitor(mExprChecker);
    assignmentStmt.RValue().AcceptVisitor(mExprChecker);
  }

  void Visit(const CompoundStmt& compoundStmt) override
  {
    compoundStmt.Recurse(*this);
  }

  void Visit(const DeclStmt& declStmt) override
  {
    const auto& varDecl = declStmt.GetVarDecl();

    if (!varDecl.HasInitExpr()) {
      return;
    }

    varDecl.InitExpr().AcceptVisitor(mExprChecker);
  }

  void Visit(const ReturnStmt& returnStmt) override
  {
    returnStmt.ReturnValue().AcceptVisitor(mExprChecker);
  }

private:
  ExprResolutionChecker mExprChecker;
};

} // namespace

bool
ResolutionCheckPass::AnalyzeVarDecl(const VarDecl& varDecl)
{
  if (!varDecl.HasInitExpr())
    return true;

  DiagErrorFilter errorFilter(GetDiagObserver());

  ExprResolutionChecker exprChecker(errorFilter);

  varDecl.InitExpr().AcceptVisitor(exprChecker);

  return !errorFilter.ErrorFlag();
}

bool
ResolutionCheckPass::AnalyzeFuncDecl(const FuncDecl& funcDecl)
{
  DiagErrorFilter errorFilter(GetDiagObserver());

  StmtResolutionChecker stmtChecker(errorFilter);

  funcDecl.AcceptBodyVisitor(stmtChecker);

  return !errorFilter.ErrorFlag();
}
