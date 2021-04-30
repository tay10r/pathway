#pragma once

#include "decl.h"
#include "decl_name.h"
#include "expr.h"
#include "location.h"
#include "module.h"
#include "stmt.h"
#include "type.h"

union SemanticValue
{
  Module* asModule;

  ModuleName* asModuleName;

  ModuleImportDecl* asModuleImportDecl;

  ModuleExportDecl* asModuleExportDecl;

  VarDecl* asVarDecl;

  ParamList* asParamList;

  FuncDecl* asFuncDecl;

  StmtList* asStmtList;

  Stmt* asStmt;

  ExprList* asExprList;

  Expr* asExpr;

  std::string* asString;

  TypeID asTypeID;

  Variability asVariability;

  Type* asType;

  uint64_t asInt;

  double asFloat;

  bool asBool;

  char32_t asInvalidChar;
};
