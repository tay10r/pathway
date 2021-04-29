#pragma once

class Diag;
class DiagObserver;
class FuncDecl;
class Program;
class VarDecl;

class AnalysisPass
{
public:
  virtual ~AnalysisPass() = default;

  bool Invoke(const Program& program, DiagObserver& diagObserver);

protected:
  void SetDiagObserver(DiagObserver* diagObserver) noexcept;

  void SetProgram(const Program* program) noexcept;

  virtual bool AnalyzeVarDecl(const VarDecl&) = 0;

  virtual bool AnalyzeFuncDecl(const FuncDecl&) = 0;

  DiagObserver& GetDiagObserver() noexcept;

  void EmitDiag(const Diag& diag);

  const Program& GetProgram() const noexcept;

private:
  DiagObserver* mDiagObserver = nullptr;

  const Program* mProgram = nullptr;
};
