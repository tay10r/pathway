#pragma once

struct Location;

class SyntaxErrorObserver
{
public:
  virtual ~SyntaxErrorObserver() = default;

  virtual void ObserveSyntaxError(const Location&, const char*) = 0;
};
