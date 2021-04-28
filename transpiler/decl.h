#pragma once

#include "decl_name.h"
#include "expr.h"
#include "location.h"
#include "stmt.h"
#include "type.h"

#include <memory>
#include <string>
#include <vector>

using ParamList = std::vector<std::unique_ptr<VarDecl>>;

class FuncDecl final
{
public:
  FuncDecl(Type* returnType, DeclName&& name, ParamList* params, Stmt* body)
    : mReturnType(returnType)
    , mName(std::move(name))
    , mParamList(params)
    , mBody(body)
  {}

  Location GetNameLocation() const noexcept { return mName.GetLocation(); }

  bool HasName(const std::string& name) const { return Identifier() == name; }

  const Type& ReturnType() const noexcept { return *mReturnType; }

  const ParamList& GetParamList() const noexcept { return *mParamList; }

  const Stmt& Body() const noexcept { return *mBody; }

  const std::string& Identifier() const noexcept { return mName.Identifier(); }

  void AcceptBodyMutator(StmtMutator& mutator)
  {
    mBody->AcceptMutator(mutator);
  }

  void AcceptBodyAccessor(StmtVisitor& visitor) const
  {
    mBody->AcceptVisitor(visitor);
  }

  bool ReferencesGlobalState() const;

  bool ReferencesFrameState() const;

  bool ReferencesPixelState() const;

  bool IsEntryPoint() const;

  bool IsPixelSampler() const;

  bool IsPixelEncoder() const;

private:
  std::unique_ptr<Type> mReturnType;

  DeclName mName;

  std::unique_ptr<ParamList> mParamList;

  std::unique_ptr<Stmt> mBody;
};

class VarDecl final
{
public:
  VarDecl(Type* type, DeclName&& name, Expr* initExpr)
    : mType(type)
    , mName(std::move(name))
    , mInitExpr(initExpr)
  {}

  const Type& GetType() const noexcept { return *mType; }

  Variability GetVariability() const noexcept
  {
    return mType->GetVariability();
  }

  TypeID GetTypeID() const noexcept { return mType->ID(); }

  std::string Identifier() const { return mName.Identifier(); }

  bool HasIdentifier(const std::string& str) const
  {
    return mName.Identifier() == str;
  }

  bool HasInitExpr() const noexcept { return !!mInitExpr; }

  bool IsGlobal() const noexcept { return mIsGlobal; }

  bool IsVaryingGlobal() const;

  bool IsUniformGlobal() const;

  void MarkAsGlobal() { mIsGlobal = true; }

  UniqueExprPtr TakeInitExpr() { return std::move(mInitExpr); }

  const Expr& InitExpr() const noexcept { return *mInitExpr; }

  Expr& InitExpr() noexcept { return *mInitExpr; }

private:
  std::unique_ptr<Type> mType;

  DeclName mName;

  UniqueExprPtr mInitExpr;

  bool mIsGlobal = false;
};
