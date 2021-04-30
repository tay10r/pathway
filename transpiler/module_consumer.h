#pragma once

#include <memory>

class Module;

class ModuleConsumer
{
public:
  virtual ~ModuleConsumer() = default;

  virtual void ConsumeModule(std::unique_ptr<Module> module) = 0;
};
