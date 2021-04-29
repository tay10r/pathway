#pragma once

/// @brief Checks for name resolution of variable references and function calls.
///
/// @return
bool
CheckResolution(const Program& program, DiagObserver& diagObserver);
