#pragma once

class Lexer;
class ProgramConsumer;
class SyntaxErrorObserver;

/// @return True on success, false on syntax error.
bool
Parse(Lexer&, ProgramConsumer&, SyntaxErrorObserver&);
