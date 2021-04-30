#pragma once

class Module;

/// @brief Resolves all variable and function references.
///
/// @detail This also includes finding the program's entry point, and uniform
/// global variables, and varying global variables.
///
/// @note Does not emit any diagnostics or "fail" in any way. Symbols that
/// aren't resolve are detecting later on.
void
Resolve(Module& module);
