#include "resolve.h"

#include "common.h"

namespace {

class Scope final
{
public:
  void Define(const Var* v) { mVarMap.emplace(*v->name.identifier, v); }

  auto FindVar(const std::string& name) const -> const Var*
  {
    auto it = mVarMap.find(name);

    return (it == mVarMap.end()) ? nullptr : it->second;
  }

private:
  std::map<std::string, const Var*> mVarMap;
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

  void Define(const Var* v)
  {
    assert(!mLocalScopes.empty());

    mLocalScopes.back().Define(v);
  }

  const Var* FindVar(const std::string& name) const
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

  void Mutate(bool_literal&) const override {}
  void Mutate(int_literal&) const override {}
  void Mutate(float_literal&) const override {}

  void Mutate(binary_expr& binaryExpr) const override
  {
    binaryExpr.Recurse(*this);
  }

  void Mutate(unary_expr& unaryExpr) const override
  {
    unaryExpr.Recurse(*this);
  }

  void Mutate(group_expr& groupExpr) const override
  {
    groupExpr.Recurse(*this);
  }

  void Mutate(var_ref& varRef) const override
  {
    const auto* var = mSymbolTable.FindVar(varRef.Identifier());

    if (!var)
      return;

    varRef.Resolve(var);
  }

  void Mutate(type_constructor& typeConstructor) const override
  {
    typeConstructor.Recurse(*this);
  }

  void Mutate(member_expr& memberExpr) const override
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

  void Mutate(compound_stmt& compoundStmt) override
  {
    mSymbolTable.EnterScope();

    compoundStmt.Recurse(*this);

    mSymbolTable.ExitScope();
  }

  void Mutate(return_stmt& returnStmt) override
  {
    ExprSymbolResolver exprSymbolResolver(mSymbolTable);

    returnStmt.return_value->AcceptMutator(exprSymbolResolver);
  }

  void Mutate(decl_stmt& s) override
  {
    if (s.v->init_expr) {

      ExprSymbolResolver exprSymbolResolver(mSymbolTable);

      s.v->init_expr->AcceptMutator(exprSymbolResolver);
    }

    mSymbolTable.Define(s.v.get());
  }

  SymbolTable mSymbolTable;
};

void
ResolveFunc(Program& program, Func& fn)
{
  SymbolTable symbolTable(program);

  symbolTable.EnterScope();

  for (const auto& p : fn.ParamList())
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
