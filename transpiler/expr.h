#pragma once

#include "decl_name.h"
#include "type.h"

#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#define _USE_MATH_DEFINES 1

#include <assert.h>
#include <math.h>
#include <stdint.h>

class VarDecl;

class BoolLiteral;
class BinaryExpr;
class FloatLiteral;
class FuncCall;
class GroupExpr;
class MemberExpr;
class IntLiteral;
class TypeConstructor;
class UnaryExpr;
class VarRef;

class ExprVisitor
{
public:
  virtual ~ExprVisitor() = default;

  virtual void Visit(const FuncCall&) = 0;

  virtual void Visit(const IntLiteral&) = 0;

  virtual void Visit(const BoolLiteral&) = 0;

  virtual void Visit(const FloatLiteral&) = 0;

  virtual void Visit(const BinaryExpr&) = 0;

  virtual void Visit(const UnaryExpr&) = 0;

  virtual void Visit(const GroupExpr&) = 0;

  virtual void Visit(const VarRef&) = 0;

  virtual void Visit(const TypeConstructor&) = 0;

  virtual void Visit(const MemberExpr&) = 0;
};

class ExprMutator
{
public:
  virtual ~ExprMutator() = default;

  virtual void Mutate(FuncCall&) const = 0;

  virtual void Mutate(IntLiteral&) const = 0;

  virtual void Mutate(BoolLiteral&) const = 0;

  virtual void Mutate(FloatLiteral&) const = 0;

  virtual void Mutate(BinaryExpr&) const = 0;

  virtual void Mutate(UnaryExpr&) const = 0;

  virtual void Mutate(GroupExpr&) const = 0;

  virtual void Mutate(VarRef&) const = 0;

  virtual void Mutate(TypeConstructor&) const = 0;

  virtual void Mutate(MemberExpr&) const = 0;
};

class Expr
{
public:
  Expr(const Location& location)
    : mLocation(location)
  {}

  virtual ~Expr() = default;

  virtual void AcceptMutator(const ExprMutator&) = 0;

  virtual void AcceptVisitor(ExprVisitor& v) const = 0;

  virtual auto GetType() const -> std::optional<Type> = 0;

  Location GetLocation() const noexcept { return mLocation; }

private:
  Location mLocation;
};

using UniqueExprPtr = std::unique_ptr<Expr>;

using ExprList = std::vector<UniqueExprPtr>;

class IntLiteral final : public Expr
{
public:
  IntLiteral(uint64_t value, const Location& location)
    : Expr(location)
    , mValue(value)
  {}

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  auto GetType() const -> std::optional<Type> override
  {
    return Type(TypeID::Int);
  }

  uint64_t Value() const noexcept { return mValue; }

private:
  uint64_t mValue;
};

class BoolLiteral final : public Expr
{
public:
  BoolLiteral(bool value, const Location& location)
    : Expr(location)
    , mValue(value)
  {}

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  auto GetType() const -> std::optional<Type> override
  {
    return Type(TypeID::Bool);
  }

  bool Value() const noexcept { return mValue; }

private:
  bool mValue;
};

class FloatLiteral final : public Expr
{
public:
  static FloatLiteral* Infinity(const Location& location)
  {
    return new FloatLiteral(std::numeric_limits<double>::infinity(), location);
  }

  static FloatLiteral* Pi(const Location& location)
  {
    return new FloatLiteral(M_PI, location);
  }

  FloatLiteral(double value, const Location& location)
    : Expr(location)
    , mValue(value)
  {}

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  auto GetType() const -> std::optional<Type> override
  {
    return Type(TypeID::Float);
  }

  double Value() const noexcept { return mValue; }

private:
  double mValue;
};

class VarRef final : public Expr
{
public:
  VarRef(const std::string& name, const Location& location)
    : Expr(location)
    , mName(DeclName(new std::string(name), location))
  {}

  VarRef(DeclName&& n)
    : Expr(n.GetLocation())
    , mName(std::move(n))
  {}

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  bool HasResolvedVar() const noexcept { return !!mResolvedVar; }

  const VarDecl& ResolvedVar() const
  {
    assert(mResolvedVar);
    return *mResolvedVar;
  }

  const std::string& Identifier() const noexcept { return mName.Identifier(); }

  void Resolve(const VarDecl* v) { mResolvedVar = v; }

  auto GetType() const -> std::optional<Type> override;

private:
  DeclName mName;

  const VarDecl* mResolvedVar = nullptr;
};

class GroupExpr final : public Expr
{
public:
  GroupExpr(Expr* e, const Location& location)
    : Expr(location)
    , mInnerExpr(e)
  {}

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  auto GetType() const -> std::optional<Type> override
  {
    return mInnerExpr->GetType();
  }

  void Recurse(const ExprMutator& mutator)
  {
    mInnerExpr->AcceptMutator(mutator);
  }

  void Recurse(ExprVisitor& Visitor) const
  {
    mInnerExpr->AcceptVisitor(Visitor);
  }

private:
  UniqueExprPtr mInnerExpr;
};

class UnaryExpr final : public Expr
{
public:
  enum class Kind
  {
    LogicalNot,
    BitwiseNot,
    Negate
  };

  UnaryExpr(Expr* expr, Kind kind, const Location& location)
    : Expr(location)
    , mBaseExpr(expr)
    , mKind(kind)
  {}

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  const Expr& BaseExpr() const noexcept { return *mBaseExpr; }

  Expr& BaseExpr() noexcept { return *mBaseExpr; }

  void Recurse(const ExprMutator& mutator)
  {
    mBaseExpr->AcceptMutator(mutator);
  }

  void Recurse(ExprVisitor& Visitor) const
  {
    mBaseExpr->AcceptVisitor(Visitor);
  }

  Kind GetKind() const noexcept { return mKind; }

  auto GetType() const -> std::optional<Type> override
  {
    return mBaseExpr->GetType();
  }

private:
  UniqueExprPtr mBaseExpr;

  Kind mKind;
};

class BinaryExpr final : public Expr
{
public:
  enum class Kind
  {
    Add,
    Sub,
    Mul,
    Div,
    Mod
  };

  BinaryExpr(Expr* left, Expr* right, Kind kind, const Location& location)
    : Expr(location)
    , mLeftExpr(left)
    , mRightExpr(right)
    , mKind(kind)
  {}

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  void Recurse(const ExprMutator& mutator)
  {
    mLeftExpr->AcceptMutator(mutator);

    mRightExpr->AcceptMutator(mutator);
  }

  void Recurse(ExprVisitor& Visitor) const
  {
    mLeftExpr->AcceptVisitor(Visitor);

    mRightExpr->AcceptVisitor(Visitor);
  }

  auto GetType() const -> std::optional<Type> override;

  const Expr& LeftExpr() const noexcept { return *mLeftExpr; }

  const Expr& RightExpr() const noexcept { return *mRightExpr; }

  Expr& LeftExpr() noexcept { return *mLeftExpr; }

  Expr& RightExpr() noexcept { return *mRightExpr; }

  Kind GetKind() const noexcept { return mKind; }

private:
  UniqueExprPtr mLeftExpr;

  UniqueExprPtr mRightExpr;

  Kind mKind;
};

class FuncDecl;

class FuncCall final : public Expr
{
public:
  FuncCall(DeclName&& name, ExprList* args, const Location& location)
    : Expr(location)
    , mName(std::move(name))
    , mArgs(args)
  {}

  void AcceptVisitor(ExprVisitor& visitor) const override
  {
    visitor.Visit(*this);
  }

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  const ExprList& Args() const noexcept { return *mArgs; }

  const std::string& Identifier() const { return mName.Identifier(); }

  const FuncDecl& GetFuncDecl() const { return *mResolvedFuncs.at(0); }

  auto GetType() const -> std::optional<Type> override;

  void QueueNameMatches(std::vector<const FuncDecl*> matches)
  {
    mResolvedFuncs = std::move(matches);
  }

  void Recurse(ExprVisitor& visitor) const
  {
    for (const auto& arg : *mArgs)
      arg->AcceptVisitor(visitor);
  }

  void Recurse(const ExprMutator& mutator)
  {
    for (auto& arg : *mArgs)
      arg->AcceptMutator(mutator);
  }

  bool Resolved() const { return mResolvedFuncs.size() == 1; }

private:
  DeclName mName;
  std::unique_ptr<ExprList> mArgs;
  /// Only one of these are going to be right, which isn't known until type
  /// coercion.
  std::vector<const FuncDecl*> mResolvedFuncs;
};

class TypeConstructor final : public Expr
{
public:
  TypeConstructor(Type type, ExprList* args, const Location& loc)
    : Expr(loc)
    , mType(type)
    , mArgs(args)
  {}

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  const ExprList& Args() const noexcept { return *mArgs; }

  ExprList& Args() noexcept { return *mArgs; }

  auto GetType() const -> std::optional<Type> override { return mType; }

  void Recurse(const ExprMutator& mutator)
  {
    for (auto& argExpr : *mArgs)
      argExpr->AcceptMutator(mutator);
  }

  void Recurse(ExprVisitor& Visitor) const
  {
    for (const auto& argExpr : *mArgs)
      argExpr->AcceptVisitor(Visitor);
  }

private:
  Type mType;

  std::unique_ptr<ExprList> mArgs;
};

class Swizzle final
{
public:
  static auto Make(const std::string& pattern, size_t sourceVectorSize)
    -> std::optional<Swizzle>;

  size_t At(size_t index) const { return mIndices.at(index); }

  size_t Size() const noexcept { return mIndices.size(); }

  auto Indices() const noexcept -> const std::vector<size_t>&
  {
    return mIndices;
  }

private:
  std::vector<size_t> mIndices;
};

class MemberExpr final : public Expr
{
public:
  MemberExpr(Expr* base_, DeclName&& member, const Location& loc)
    : Expr(loc)
    , mBaseExpr(base_)
    , mMemberName(std::move(member))
  {}

  void AcceptVisitor(ExprVisitor& v) const override { v.Visit(*this); }

  void AcceptMutator(const ExprMutator& mutator) override
  {
    mutator.Mutate(*this);
  }

  const Expr& BaseExpr() const noexcept { return *mBaseExpr; }

  auto GetType() const -> std::optional<Type> override;

  const DeclName& MemberName() const noexcept { return mMemberName; }

  void Recurse(const ExprMutator& mutator)
  {
    mBaseExpr->AcceptMutator(mutator);
  }

  void Recurse(ExprVisitor& Visitor) const
  {
    mBaseExpr->AcceptVisitor(Visitor);
  }

private:
  UniqueExprPtr mBaseExpr;

  DeclName mMemberName;
};
