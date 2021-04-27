#pragma once

#include <memory>

class Program;

class ProgramConsumer
{
public:
  virtual ~ProgramConsumer() = default;

  virtual void ConsumeProgram(std::unique_ptr<Program> program) = 0;
};
