#include "resolve.h"

#include "program.h"

#include <map>

namespace {

class Scope final
{
public:
  void Define(const VarDecl* v) { mVarMap.emplace(v->Identifier(), v); }

  auto FindVar(const std::string& name) const -> const VarDecl*
  {
    auto it = mVarMap.find(name);

    return (it == mVarMap.end()) ? nullptr : it->second;
  }

private:
  std::map<std::string, const VarDecl*> mVarMap;
};

class SymbolTable final
{
public:
  SymbolTable(const Program& program)
    : mProgram(program)
  {}

  void EnterScope() { mLocalScopes.emplace_back(); }

  void ExitScope()
  {
    assert(!mLocalScopes.empty());

    mLocalScopes.pop_back();
  }

  void Define(const VarDecl* v)
  {
    assert(!mLocalScopes.empty());

    mLocalScopes.back().Define(v);
  }

  auto FindFuncs(const std::string& name) const -> std::vector<const FuncDecl*>
  {
    std::vector<const FuncDecl*> matches;

    for (const auto& func : mProgram.Funcs()) {
      if (func->HasName(name))
        matches.emplace_back(func.get());
    }

    return matches;
  }

  const VarDecl* FindVar(const std::string& name) const
  {
    for (auto it = mLocalScopes.rbegin(); it != mLocalScopes.rend(); it++) {

      const auto* var = it->FindVar(name);
      if (var)
        return var;
    }

    for (const auto& var : mProgram.GlobalVars()) {
      if (var->HasIdentifier(name))
        return var.get();
    }

    return nullptr;
  }

private:
  std::vector<Scope> mLocalScopes;

  /// @note This is used for resolving global declarations.
  const Program& mProgram;
};

class ExprSymbolResolver final : public ExprMutator
{
public:
  ExprSymbolResolver(const SymbolTable& symbolTable)
    : mSymbolTable(symbolTable)
  {}

  void Mutate(BoolLiteral&) const override {}
  void Mutate(IntLiteral&) const override {}
  void Mutate(FloatLiteral&) const override {}

  void Mutate(BinaryExpr& binaryExpr) const override
  {
    binaryExpr.Recurse(*this);
  }

  void Mutate(FuncCall& funcCall) const override
  {
    funcCall.QueueNameMatches(mSymbolTable.FindFuncs(funcCall.Identifier()));

    funcCall.Recurse(*this);
  }

  void Mutate(UnaryExpr& unaryExpr) const override { unaryExpr.Recurse(*this); }

  void Mutate(GroupExpr& groupExpr) const override { groupExpr.Recurse(*this); }

  void Mutate(VarRef& varRef) const override
  {
    const auto* var = mSymbolTable.FindVar(varRef.Identifier());

    if (!var)
      return;

    varRef.Resolve(var);
  }

  void Mutate(TypeConstructor& typeConstructor) const override
  {
    typeConstructor.Recurse(*this);
  }

  void Mutate(MemberExpr& memberExpr) const override
  {
    memberExpr.Recurse(*this);
  }

private:
  const SymbolTable& mSymbolTable;
};

class StmtSymbolResolver final : public StmtMutator
{
public:
  StmtSymbolResolver(SymbolTable& symbolTable)
    : mSymbolTable(symbolTable)
  {}

private:
  void Mutate(AssignmentStmt& assignmentStmt) override
  {
    ExprSymbolResolver exprSymbolResolver(mSymbolTable);

    assignmentStmt.LValue().AcceptMutator(exprSymbolResolver);
    assignmentStmt.RValue().AcceptMutator(exprSymbolResolver);
  }

  void Mutate(CompoundStmt& compoundStmt) override
  {
    mSymbolTable.EnterScope();

    compoundStmt.Recurse(*this);

    mSymbolTable.ExitScope();
  }

  void Mutate(ReturnStmt& returnStmt) override
  {
    ExprSymbolResolver exprSymbolResolver(mSymbolTable);

    returnStmt.ReturnValue().AcceptMutator(exprSymbolResolver);
  }

  void Mutate(DeclStmt& declStmt) override
  {
    if (declStmt.GetVarDecl().HasInitExpr()) {

      ExprSymbolResolver exprSymbolResolver(mSymbolTable);

      declStmt.GetVarDecl().InitExpr().AcceptMutator(exprSymbolResolver);
    }

    mSymbolTable.Define(&declStmt.GetVarDecl());
  }

  SymbolTable mSymbolTable;
};

void
ResolveFunc(Program& program, FuncDecl& fn)
{
  SymbolTable symbolTable(program);

  symbolTable.EnterScope();

  for (const auto& p : fn.GetParamList())
    symbolTable.Define(p.get());

  StmtSymbolResolver resolver(symbolTable);

  fn.AcceptBodyMutator(resolver);

  symbolTable.ExitScope();
}

} // namespace

void
Resolve(Program& program)
{
  for (auto& fn : program.Funcs()) {
    ResolveFunc(program, *fn);
  }
}
