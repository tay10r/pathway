#pragma once

class Diag;
class DiagObserver;
class FuncDecl;
class Module;
class VarDecl;

class AnalysisPass
{
public:
  virtual ~AnalysisPass() = default;

  bool Invoke(const Module& module, DiagObserver& diagObserver);

protected:
  void SetDiagObserver(DiagObserver* diagObserver) noexcept;

  void SetModule(const Module* module) noexcept;

  /// @brief Iterates all the variables and functions.
  /// Can be overriden to do custom whole-program analysis.
  virtual bool AnalyzeModule();

  virtual bool AnalyzeVarDecl(const VarDecl&) = 0;

  virtual bool AnalyzeFuncDecl(const FuncDecl&) = 0;

  DiagObserver& GetDiagObserver() noexcept;

  void EmitDiag(const Diag& diag);

  const Module& GetModule() const noexcept;

private:
  DiagObserver* mDiagObserver = nullptr;

  const Module* mModule = nullptr;
};
