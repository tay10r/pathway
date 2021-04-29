#pragma once

#include "expr.h"
#include "type.h"

#include <memory>
#include <vector>

class AssignmentStmt;
class CompoundStmt;
class DeclStmt;
class ReturnStmt;

class StmtVisitor
{
public:
  virtual ~StmtVisitor() = default;

  virtual void Visit(const AssignmentStmt&) = 0;

  virtual void Visit(const CompoundStmt&) = 0;

  virtual void Visit(const DeclStmt&) = 0;

  virtual void Visit(const ReturnStmt&) = 0;
};

class StmtMutator
{
public:
  virtual ~StmtMutator() {}

  virtual void Mutate(AssignmentStmt&) = 0;

  virtual void Mutate(CompoundStmt&) = 0;

  virtual void Mutate(DeclStmt&) = 0;

  virtual void Mutate(ReturnStmt&) = 0;
};

class Stmt
{
public:
  virtual ~Stmt() = default;

  virtual void AcceptVisitor(StmtVisitor& v) const = 0;

  virtual void AcceptMutator(StmtMutator& m) = 0;
};

using UniqueStmtPtr = std::unique_ptr<Stmt>;

using StmtList = std::vector<std::unique_ptr<Stmt>>;

class AssignmentStmt final : public Stmt
{
public:
  AssignmentStmt(Expr* lValue, Expr* rValue)
    : mLValue(lValue)
    , mRValue(rValue)
  {}

  void AcceptVisitor(StmtVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(StmtMutator& m) override { m.Mutate(*this); }

  Expr& LValue() noexcept { return *mLValue; }
  Expr& RValue() noexcept { return *mRValue; }

  const Expr& LValue() const noexcept { return *mLValue; }
  const Expr& RValue() const noexcept { return *mRValue; }

private:
  UniqueExprPtr mLValue;
  UniqueExprPtr mRValue;
};

class CompoundStmt final : public Stmt
{
public:
  CompoundStmt(StmtList* stmts_)
    : stmts(stmts_)
  {}

  void AcceptVisitor(StmtVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(StmtMutator& m) override { m.Mutate(*this); }

  void Recurse(StmtVisitor& v) const
  {
    for (auto& innerStmt : *stmts)
      innerStmt->AcceptVisitor(v);
  }

  void Recurse(StmtMutator& m)
  {
    for (auto& innerStmt : *stmts)
      innerStmt->AcceptMutator(m);
  }

private:
  std::unique_ptr<StmtList> stmts;
};

class VarDecl;

class DeclStmt final : public Stmt
{
public:
  DeclStmt(VarDecl* v_);

  DeclStmt(DeclStmt&&);

  ~DeclStmt();

  VarDecl& GetVarDecl() noexcept { return *mVarDecl; }

  const VarDecl& GetVarDecl() const noexcept { return *mVarDecl; }

  void AcceptVisitor(StmtVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(StmtMutator& m) override { m.Mutate(*this); }

private:
  std::unique_ptr<VarDecl> mVarDecl;
};

class ReturnStmt final : public Stmt
{
public:
  ReturnStmt(Expr* rv)
    : mReturnValue(rv)
  {}

  void AcceptVisitor(StmtVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(StmtMutator& m) override { m.Mutate(*this); }

  const Expr& ReturnValue() const noexcept { return *mReturnValue; }

  Expr& ReturnValue() noexcept { return *mReturnValue; }

private:
  UniqueExprPtr mReturnValue;
};
