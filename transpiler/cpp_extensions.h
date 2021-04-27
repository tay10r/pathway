#pragma once

#include <string>

#include <stddef.h>

namespace cpp {

class DataMode
{
public:
  virtual ~DataMode() = default;

  virtual size_t ScalarBits() const noexcept = 0;

  /// @note Should be compatible with ISPC targets, if possible. Also should be
  /// a valid C identifier.
  virtual std::string ModeID() const = 0;

  virtual std::string EmitIntLiteral(uint64_t value) const = 0;
};

class I32_DataMode final : public DataMode
{
public:
  size_t ScalarBits() const noexcept override { return 32; }

  std::string ModeID() const override { return "i32"; };

  std::string EmitIntLiteral(uint64_t value) const override
  {
    return std::to_string(value);
  }
};

class I64_DataMode final : public DataMode
{
public:
  size_t ScalarBits() const noexcept override { return 64; }

  std::string ModeID() const override { return "i64"; };

  std::string EmitIntLiteral(uint64_t value) const override
  {
    return std::to_string(value) + "ll";
  }
};

class SSE_I32_BaseDataMode : public DataMode
{
public:
  virtual ~SSE_I32_BaseDataMode() = default;

  size_t ScalarBits() const noexcept override { return 32; }

  std::string EmitIntLiteral(uint64_t value) const override
  {
    std::ostringstream stream;
    stream << "_mm_set1_epi32(";
    stream << value;
    stream << ")";
    return stream.str();
  }
};

class SSE2_I32_DataMode final : public SSE_I32_BaseDataMode
{
public:
  std::string ModeID() const override { return "sse2_i32x4"; }
};

} // namespace cpp
