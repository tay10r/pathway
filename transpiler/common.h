#pragma once

#include "decl_name.h"
#include "expr.h"
#include "location.h"
#include "stmt.h"
#include "type.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

#include <assert.h>
#include <stdint.h>

using param_list = std::vector<std::unique_ptr<Var>>;

class Func final
{
public:
  Func(Type* returnType, DeclName&& name, param_list* params, Stmt* body)
    : mReturnType(returnType)
    , mName(std::move(name))
    , mParamList(params)
    , mBody(body)
  {}

  Location GetNameLocation() const noexcept { return mName.GetLocation(); }

  bool HasName(const std::string& name) const { return Identifier() == name; }

  const Type& ReturnType() const noexcept { return *mReturnType; }

  const param_list& ParamList() const noexcept { return *mParamList; }

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

  std::unique_ptr<param_list> mParamList;

  std::unique_ptr<Stmt> mBody;
};

struct Var final
{
  std::unique_ptr<Type> mType;

  DeclName name;

  UniqueExprPtr init_expr;

  Var(Type* type, DeclName&& n, Expr* e)
    : mType(type)
    , name(std::move(n))
    , init_expr(e)
  {}

  Variability GetVariability() const noexcept
  {
    return mType->GetVariability();
  }

  TypeID GetTypeID() const noexcept { return mType->ID(); }

  std::string Identifier() const { return name.Identifier(); }

  bool HasIdentifier(const std::string& str) const
  {
    return name.Identifier() == str;
  }

  bool IsGlobal() const noexcept { return mIsGlobal; }

  bool IsVaryingGlobal() const;

  bool IsUniformGlobal() const;

  void MarkAsGlobal() { mIsGlobal = true; }

  UniqueExprPtr TakeInitExpr() { return std::move(init_expr); }

private:
  bool mIsGlobal = false;
};

class Program final
{
public:
  void AppendFunc(Func* f) { mFuncs.emplace_back(f); }

  void AppendGlobalVar(Var* globalVar);

  const auto& Funcs() const noexcept { return mFuncs; }

  const auto& GlobalVars() const noexcept { return mGlobalVars; }

  const auto& UniformGlobalVars() const noexcept { return mUniformGlobalVars; }

  const auto& VaryingGlobalVars() const noexcept { return mVaryingGlobalVars; }

private:
  std::vector<std::unique_ptr<Func>> mFuncs;

  std::vector<std::unique_ptr<Var>> mGlobalVars;

  std::vector<const Var*> mVaryingGlobalVars;

  std::vector<const Var*> mUniformGlobalVars;
};

union semantic_value
{
  Program* asProgram;

  Var* as_var;

  param_list* as_param_list;

  Func* as_func;

  StmtList* as_stmt_list;

  Stmt* as_stmt;

  ExprList* as_expr_list;

  Expr* as_expr;

  std::string* as_string;

  TypeID asTypeID;

  Variability asVariability;

  Type* asType;

  uint64_t as_int;

  double as_float;

  bool asBool;

  char32_t invalid_char;
};
