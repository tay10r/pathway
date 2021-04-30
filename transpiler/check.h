#pragma once

#include <iosfwd>

class Module;

bool
check(const std::string& path, const Module& module, std::ostream& os);
