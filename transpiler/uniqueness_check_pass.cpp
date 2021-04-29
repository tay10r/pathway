#include "uniqueness_check_pass.h"

#include "decl.h"
#include "diagnostics.h"
#include "expr.h"

namespace {

// TODO:
//  [ ] global variables
//  [ ] functions
//  [ ] parameter variables
//  [ ] local variables

enum class SymbolKind
{
  Func,
  Var
};

struct Symbol final
{
  SymbolKind mKind;

  Location mLocation;
};

using Scope = std::map<std::string, Symbol>;

using SymbolTable = std::vector<Scope>;

class StmtUniquenessChecker final : public StmtVisitor
{
public:
  StmtUniquenessChecker(DiagErrorFilter& errorFilter)
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

    varDecl.InitExpr().AcceptVisitor(mExprChecker);
  }

  void Visit(const ReturnStmt& returnStmt) override
  {
    returnStmt.ReturnValue().AcceptVisitor(mExprChecker);
  }

private:
  ExprUniquenessChecker mExprChecker;
};

} // namespace

bool
UniquenessCheckPass::AnalyzeVarDecl(const VarDecl& varDecl)
{
  if (!varDecl.HasInitExpr())
    return true;

  DiagErrorFilter errorFilter(GetDiagObserver());

  ExprUniquenessChecker exprChecker(errorFilter);

  varDecl.InitExpr().AcceptVisitor(exprChecker);

  return !errorFilter.ErrorFlag();
}

bool
UniquenessCheckPass::AnalyzeFuncDecl(const FuncDecl& funcDecl)
{
  DiagErrorFilter errorFilter(GetDiagObserver());

  StmtUniquenessChecker stmtChecker(errorFilter);

  funcDecl.AcceptBodyVisitor(stmtChecker);

  return !errorFilter.ErrorFlag();
}
