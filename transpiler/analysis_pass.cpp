#include "analysis_pass.h"

#include "abort.h"
#include "diagnostics.h"
#include "module.h"

void
AnalysisPass::SetDiagObserver(DiagObserver* diagObserver) noexcept
{
  mDiagObserver = diagObserver;
}

void
AnalysisPass::SetModule(const Module* module) noexcept
{
  mModule = module;
}

bool
AnalysisPass::Invoke(const Module& module, DiagObserver& diagObserver)
{
  SetModule(&module);

  SetDiagObserver(&diagObserver);

  auto success = AnalyzeModule();

  SetModule(nullptr);

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

const Module&
AnalysisPass::GetModule() const noexcept
{
  if (!mModule) {
    ABORT("Could not pass module to analyzer because it is null.");
  }

  return *mModule;
}

bool
AnalysisPass::AnalyzeModule()
{
  const auto& module = GetModule();

  bool success = true;

  for (const auto& var : module.GlobalVars()) {
    success &= AnalyzeVarDecl(*var);
  }

  for (const auto& func : module.Funcs()) {
    success &= AnalyzeFuncDecl(*func);
  }

  return success;
}

void
AnalysisPass::EmitDiag(const Diag& diag)
{
  if (!mDiagObserver) {
    ABORT("Could not emit diagnostic because observer is null.");
  }

  mDiagObserver->Observe(diag);
}
