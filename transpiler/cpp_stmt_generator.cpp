#include "cpp_stmt_generator.h"

#include "cpp_expr_environment_impl.h"
#include "cpp_expr_generator.h"

// for TypePrinter
#include "cpp_generator_v2.h"

namespace cpp {

std::string
StmtGenerator::String() const
{
  return mStream.str();
}

void
StmtGenerator::Visit(const AssignmentStmt& assignmentStmt)
{
  ExprEnvironmentImpl exprEnv(mProgram);

  ExprGenerator lExprGen(exprEnv);
  ExprGenerator rExprGen(exprEnv);

  assignmentStmt.LValue().AcceptVisitor(lExprGen);
  assignmentStmt.RValue().AcceptVisitor(rExprGen);

  Indent() << lExprGen.String() << " = " << rExprGen.String() << ';'
           << std::endl;
}

void
StmtGenerator::Visit(const CompoundStmt& compoundStmt)
{
  Indent() << '{' << std::endl;

  mIndentLevel++;

  compoundStmt.Recurse(*this);

  mIndentLevel--;

  Indent() << '}' << std::endl;
}

void
StmtGenerator::Visit(const DeclStmt& declStmt)
{
  const auto& varDecl = declStmt.GetVarDecl();

  TypePrinter typePrinter;

  typePrinter.Visit(varDecl.GetType());

  Indent() << typePrinter.String() << ' ' << varDecl.Identifier();

  if (varDecl.HasInitExpr()) {

    ExprEnvironmentImpl exprEnv(mProgram);

    ExprGenerator exprGenerator(exprEnv);

    varDecl.InitExpr().AcceptVisitor(exprGenerator);

    mStream << " = " << exprGenerator.String();
  }

  mStream << ';' << std::endl;
}

void
StmtGenerator::Visit(const ReturnStmt& returnStmt)
{
  Indent() << "return ";

  ExprEnvironmentImpl exprEnv(mProgram);

  ExprGenerator exprGenerator(exprEnv);

  returnStmt.ReturnValue().AcceptVisitor(exprGenerator);

  mStream << exprGenerator.String() << ';' << std::endl;
}

std::ostream&
StmtGenerator::Indent()
{
  for (size_t i = 0; i < mIndentLevel; i++)
    mStream << "  ";

  return mStream;
}

} // namespace cpp
