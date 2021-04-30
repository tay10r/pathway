#pragma once

class Lexer;
class ModuleConsumer;
class SyntaxErrorObserver;

/// @return True on success, false on syntax error.
bool
Parse(Lexer&, ModuleConsumer&, SyntaxErrorObserver&);
