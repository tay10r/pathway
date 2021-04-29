#pragma once

#include "analysis_pass.h"

class UniquenessCheckPass final : public AnalysisPass
{
public:
  bool AnalyzeVarDecl(const VarDecl&) override;

  bool AnalyzeFuncDecl(const FuncDecl&) override;
};
