#include "stmt.h"

#include "decl.h"

DeclStmt::DeclStmt(VarDecl* v_)
  : mVarDecl(v_)
{}

DeclStmt::DeclStmt(DeclStmt&&) = default;

DeclStmt::~DeclStmt() = default;
