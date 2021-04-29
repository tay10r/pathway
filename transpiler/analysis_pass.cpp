#include "analysis_pass.h"

#include "abort.h"
#include "diagnostics.h"
#include "program.h"

void
AnalysisPass::SetDiagObserver(DiagObserver* diagObserver) noexcept
{
  mDiagObserver = diagObserver;
}

void
AnalysisPass::SetProgram(const Program* program) noexcept
{
  mProgram = program;
}

bool
AnalysisPass::Invoke(const Program& program, DiagObserver& diagObserver)
{
  SetProgram(&program);

  SetDiagObserver(&diagObserver);

  bool success = true;

  for (const auto& var : program.GlobalVars()) {
    success &= AnalyzeVarDecl(*var);
  }

  for (const auto& func : program.Funcs()) {
    success &= AnalyzeFuncDecl(*func);
  }

  SetProgram(nullptr);

  SetDiagObserver(nullptr);

  return success;
}

DiagObserver&
AnalysisPass::GetDiagObserver() noexcept
{
  if (!mDiagObserver) {
    ABORT("Could not pass diagnostic observer to analyzer because it is null.");
  }

  return *mDiagObserver;
}

const Program&
AnalysisPass::GetProgram() const noexcept
{
  if (!mProgram) {
    ABORT("Could not pass program to analyzer because it is null.");
  }

  return *mProgram;
}

void
AnalysisPass::EmitDiag(const Diag& diag)
{
  if (!mDiagObserver) {
    ABORT("Could not emit diagnostic because observer is null.");
  }

  mDiagObserver->Observe(diag);
}
