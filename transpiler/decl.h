#pragma once

#include "abort.h"
#include "decl_name.h"
#include "expr.h"
#include "location.h"
#include "stmt.h"
#include "type.h"

#include <memory>
#include <string>
#include <vector>

class FuncDecl;
class VarDecl;
class ModuleExportDecl;
class ModuleImportDecl;

class DeclVisitor
{
public:
  virtual ~DeclVisitor() = default;

  virtual void Visit(const FuncDecl&) = 0;

  /// @note Only applies to global variables.
  virtual void Visit(const VarDecl&) = 0;

  virtual void Visit(const ModuleExportDecl&) = 0;

  virtual void Visit(const ModuleImportDecl&) = 0;
};

class DeclMutator
{
public:
  virtual ~DeclMutator() = default;

  virtual void Mutate(FuncDecl&) = 0;

  /// @note Only applies to global variables.
  virtual void Mutate(VarDecl&) = 0;

  virtual void Mutate(ModuleExportDecl&) = 0;

  virtual void Mutate(ModuleImportDecl&) = 0;
};

class Decl
{
public:
  virtual ~Decl() = default;

  virtual void AcceptVisitor(DeclVisitor&) const = 0;

  virtual void AcceptMutator(DeclMutator&) = 0;
};

using UniqueDeclPtr = std::unique_ptr<Decl>;

using DeclList = std::vector<UniqueDeclPtr>;

class ModuleName final
{
public:
  void Append(std::string* identifier, const Location& location);

  std::string ToSingleIdentifier() const;

  auto Identifiers() const noexcept
    -> const std::vector<std::unique_ptr<std::string>>&
  {
    return mIdentifiers;
  }

private:
  std::vector<std::unique_ptr<std::string>> mIdentifiers;

  std::vector<Location> mLocations;
};

class ModuleExportDecl final : public Decl
{
public:
  ModuleExportDecl(ModuleName* name)
    : mName(name)
  {}

  void AcceptVisitor(DeclVisitor& visitor) const override
  {
    visitor.Visit(*this);
  }

  void AcceptMutator(DeclMutator& mutator) override { mutator.Mutate(*this); }

  bool HasModuleName() const noexcept { return !!mName; }

  const ModuleName& GetModuleName() const noexcept
  {
    if (!mName) {
      ABORT("Module name was accessed, but is null.");
    }

    return *mName;
  }

private:
  std::unique_ptr<ModuleName> mName;
};

class ModuleImportDecl final : public Decl
{
public:
  ModuleImportDecl(ModuleName* name)
    : mName(name)
  {}

  void AcceptVisitor(DeclVisitor& visitor) const override
  {
    visitor.Visit(*this);
  }

  void AcceptMutator(DeclMutator& mutator) override { mutator.Mutate(*this); }

  const DeclList& Body() const noexcept { return *mBody; }

  bool HasBody() const noexcept { return !mBody; }

  const ModuleName& GetModuleName() const noexcept
  {
    if (!mName) {
      ABORT("Module name was accessed, but is null.");
    }

    return *mName;
  }

private:
  std::unique_ptr<ModuleName> mName;

  std::unique_ptr<DeclList> mBody;
};

using ParamList = std::vector<std::unique_ptr<VarDecl>>;

class FuncDecl final : public Decl
{
public:
  FuncDecl(Type* returnType, DeclName&& name, ParamList* params, Stmt* body)
    : mReturnType(returnType)
    , mName(std::move(name))
    , mParamList(params)
    , mBody(body)
  {}

  void AcceptVisitor(DeclVisitor& visitor) const override
  {
    visitor.Visit(*this);
  }

  void AcceptMutator(DeclMutator& mutator) override { mutator.Mutate(*this); }

  Location GetNameLocation() const noexcept { return mName.GetLocation(); }

  std::string GetMangledName() const;

  bool HasName(const std::string& name) const { return Identifier() == name; }

  const Type& ReturnType() const noexcept { return *mReturnType; }

  const ParamList& GetParamList() const noexcept { return *mParamList; }

  const Stmt& Body() const noexcept { return *mBody; }

  const std::string& Identifier() const noexcept { return mName.Identifier(); }

  void AcceptBodyMutator(StmtMutator& mutator)
  {
    mBody->AcceptMutator(mutator);
  }

  void AcceptBodyVisitor(StmtVisitor& visitor) const
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

class VarDecl final : public Decl
{
public:
  VarDecl(Type* type, DeclName&& name, Expr* initExpr)
    : mType(type)
    , mName(std::move(name))
    , mInitExpr(initExpr)
  {}

  void AcceptVisitor(DeclVisitor& visitor) const override
  {
    visitor.Visit(*this);
  }

  void AcceptMutator(DeclMutator& mutator) override { mutator.Mutate(*this); }

  const Type& GetType() const noexcept { return *mType; }

  Variability GetVariability() const noexcept
  {
    return mType->GetVariability();
  }

  Location GetNameLocation() const noexcept { return mName.GetLocation(); }

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
