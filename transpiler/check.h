#pragma once

#include <iosfwd>

class Program;

bool
check(const std::string& path, const Program& program, std::ostream& os);
