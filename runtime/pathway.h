#pragma once

#ifndef PATHWAY_COMMON_RUNTIME_H_INCLUDED
#define PATHWAY_COMMON_RUNTIME_H_INCLUDED

#include <utility>
#include <vector>

#include <stddef.h>

namespace pathway {

// {{{ Generic Operations
//=======================

template<typename scalar, size_t>
class vector;

template<size_t count, size_t index = 0>
struct binary_op final
{
  // vector & vector and matrix & matrix operations

  template<typename operand>
  static void add(const operand& a, const operand& b, operand& out) noexcept
  {
    out.template at<index>() = a.template at<index>() + b.template at<index>();

    binary_op<count, index + 1>::add(a, b, out);
  }

  template<typename operand>
  static void sub(const operand& a, const operand& b, operand& out) noexcept
  {
    out.template at<index>() = a.template at<index>() - b.template at<index>();

    binary_op<count, index + 1>::sub(a, b, out);
  }

  // vector & scalar operations

  template<typename scalar, size_t vector_size>
  static void add(const vector<scalar, vector_size>& a,
                  scalar b,
                  vector<scalar, vector_size>& out) noexcept
  {
    out.template at<index>() = a.template at<index>() + b;

    binary_op<count, index + 1>::add(a, b, out);
  }

  template<typename scalar, size_t vector_size>
  static void sub(const vector<scalar, vector_size>& a,
                  scalar b,
                  vector<scalar, vector_size>& out) noexcept
  {
    out.template at<index>() = a.template at<index>() - b;

    binary_op<count, index + 1>::sub(a, b, out);
  }

  template<typename scalar, size_t vector_size>
  static void mul(const vector<scalar, vector_size>& a,
                  scalar b,
                  vector<scalar, vector_size>& out) noexcept
  {
    out.template at<index>() = a.template at<index>() * b;

    binary_op<count, index + 1>::mul(a, b, out);
  }

  template<typename scalar, size_t vector_size>
  static void div(const vector<scalar, vector_size>& a,
                  scalar b,
                  vector<scalar, vector_size>& out) noexcept
  {
    out.template at<index>() = a.template at<index>() / b;

    binary_op<count, index + 1>::div(a, b, out);
  }

#if 0
  template <typename operand>
  static void min(const operand& a, const operand& b, operand& out) noexcept
  {
  }
#endif
};

template<size_t index>
struct binary_op<index, index> final
{
  template<typename operand>
  static void add(const operand&, const operand&, operand&) noexcept
  {}

  template<typename operand>
  static void sub(const operand&, const operand&, operand&) noexcept
  {}

  template<typename scalar, size_t vector_size>
  static void add(const vector<scalar, vector_size>&,
                  scalar,
                  vector<scalar, vector_size>&) noexcept
  {}

  template<typename scalar, size_t vector_size>
  static void sub(const vector<scalar, vector_size>&,
                  scalar,
                  vector<scalar, vector_size>&) noexcept
  {}

  template<typename scalar, size_t vector_size>
  static void mul(const vector<scalar, vector_size>&,
                  scalar,
                  vector<scalar, vector_size>&) noexcept
  {}

  template<typename scalar, size_t vector_size>
  static void div(const vector<scalar, vector_size>&,
                  scalar,
                  vector<scalar, vector_size>&) noexcept
  {}
};

template<typename scalar>
scalar
min(scalar x, scalar y) noexcept
{
  return (x < y) ? x : y;
}

template<typename scalar>
scalar
max(scalar x, scalar y) noexcept
{
  return (x < y) ? y : x;
}

template<typename scalar>
scalar
clamp(scalar x, scalar min_value, scalar max_value) noexcept
{
  return max(min(x, max_value), min_value);
}

//=======================
// }}} Generic Operations

// {{{ Builtin Types
//==================

// {{{ Vector
//===========

template<typename scalar, size_t size>
class vector final
{
public:
  template<size_t index>
  scalar& at() noexcept
  {
    static_assert(index < size, "Vector element index is out of bounds.");
    return data[index];
  }

  template<size_t index>
  const scalar& at() const noexcept
  {
    static_assert(index < size, "Vector element index is out of bounds.");
    return data[index];
  }

private:
  scalar data[size];
};

template<typename scalar>
using vec2 = vector<scalar, 2>;

template<typename scalar>
using vec3 = vector<scalar, 3>;

template<typename scalar>
using vec4 = vector<scalar, 4>;

template<typename scalar>
auto
make_vec2(scalar x, scalar y) noexcept -> vec2<scalar>
{
  vec2<scalar> v;
  v.template at<0>() = x;
  v.template at<1>() = y;
  return v;
};

template<typename scalar>
auto
make_vec3(scalar x, scalar y, scalar z) noexcept -> vec3<scalar>
{
  vec3<scalar> v;
  v.template at<0>() = x;
  v.template at<1>() = y;
  v.template at<2>() = z;
  return v;
}

template<typename scalar>
auto
make_vec4(scalar x, scalar y, scalar z, scalar w) noexcept -> vec4<scalar>
{
  vec4<scalar> v;
  v.template at<0>() = x;
  v.template at<1>() = y;
  v.template at<2>() = z;
  v.template at<3>() = w;
  return v;
}

template<typename scalar, size_t size>
auto
operator+(const vector<scalar, size>& a, const vector<scalar, size>& b) noexcept
  -> vector<scalar, size>
{
  vector<scalar, size> out;

  binary_op<size>::add(a, b, out);

  return out;
}

template<typename scalar, size_t size>
auto
operator-(const vector<scalar, size>& a, const vector<scalar, size>& b) noexcept
  -> vector<scalar, size>
{
  vector<scalar, size> out;

  binary_op<size>::sub(a, b, out);

  return out;
}

template<typename scalar, size_t size>
auto
operator+(const vector<scalar, size>& a, scalar b) noexcept
  -> vector<scalar, size>
{
  vector<scalar, size> out;

  binary_op<size>::add(a, b, out);

  return out;
}

template<typename scalar, size_t size>
auto
operator-(const vector<scalar, size>& a, scalar b) noexcept
  -> vector<scalar, size>
{
  vector<scalar, size> out;

  binary_op<size>::sub(a, b, out);

  return out;
}

template<typename scalar, size_t size>
auto
operator*(const vector<scalar, size>& a, scalar b) noexcept
  -> vector<scalar, size>
{
  vector<scalar, size> out;

  binary_op<size>::mul(a, b, out);

  return out;
}

template<typename scalar, size_t size>
auto
operator/(const vector<scalar, size>& a, scalar b) noexcept
  -> vector<scalar, size>
{
  vector<scalar, size> out;

  binary_op<size>::div(a, b, out);

  return out;
}

template<typename scalar, size_t size>
auto
operator+(scalar a, const vector<scalar, size>& b) noexcept
  -> vector<scalar, size>
{
  return b + a;
}

template<typename scalar, size_t size>
auto
operator-(scalar a, const vector<scalar, size>& b) noexcept
  -> vector<scalar, size>
{
  return b - a;
}

template<typename scalar, size_t size>
auto
operator*(scalar a, const vector<scalar, size>& b) noexcept
  -> vector<scalar, size>
{
  return b * a;
}

template<typename scalar, size_t size>
auto
operator/(scalar a, const vector<scalar, size>& b) noexcept
  -> vector<scalar, size>
{
  return b / a;
}

// {{{ Constructor
//================

template<size_t index, size_t size, size_t other_index, size_t other_size>
struct vector_blit final
{
  template<typename scalar>
  static void blit(vector<scalar, size>& dst,
                   const vector<scalar, other_size>& src) noexcept
  {
    dst.template at<index>() = src.template at<other_index>();

    vector_blit<index + 1, size, other_index + 1, other_size>::blit(dst, src);
  }
};

template<size_t index, size_t size, size_t other_size>
struct vector_blit<index, size, other_size, other_size> final
{
  template<typename scalar>
  static void blit(vector<scalar, size>&,
                   const vector<scalar, other_size>&) noexcept
  {}
};

template<size_t index, size_t size>
struct vector_initializer final
{
  template<typename scalar, size_t vec_size>
  static void init_uniform(vector<scalar, vec_size>& v, scalar n) noexcept
  {
    v.template at<index>() = n;

    vector_initializer<index + 1, size>::init_uniform(v, n);
  }

  template<typename scalar, typename... args>
  static void init(vector<scalar, size>& out,
                   scalar n,
                   const args&... other_args) noexcept
  {
    out.template at<index>() = n;

    vector_initializer<index + 1, size>::init(out, other_args...);
  }

  template<typename scalar, size_t other_vector_size, typename... args>
  static void init(vector<scalar, size>& out,
                   const vector<scalar, other_vector_size>& other,
                   const args&... other_args)
  {
    vector_blit<index, size, 0, other_vector_size>::blit(out, other);

    vector_initializer<index + other_vector_size, size>::init(out,
                                                              other_args...);
  }

  template<typename scalar>
  static void init(vector<scalar, size>&) noexcept
  {}
};

template<size_t size>
struct vector_initializer<size, size> final
{
  template<typename scalar, size_t vec_size>
  static void init_uniform(vector<scalar, vec_size>&, scalar) noexcept
  {}

  template<typename scalar>
  static void init(vector<scalar, size>&) noexcept
  {}
};

template<size_t size>
struct vector_constructor final
{
  template<typename scalar>
  static auto make(scalar n) noexcept -> vector<scalar, size>
  {
    vector<scalar, size> v;

    vector_initializer<0, size>::init_uniform(v, n);

    return v;
  }

  template<typename scalar, typename... other_args>
  static auto make(scalar first, const other_args&... args) noexcept
    -> vector<scalar, size>
  {
    vector<scalar, size> out;

    vector_initializer<0, size>::init(out, first, args...);

    return out;
  }

  template<typename scalar, size_t any_size, typename... other_args>
  static auto make(const vector<scalar, any_size>& first,
                   const other_args&... args) noexcept -> vector<scalar, size>
  {
    vector<scalar, size> out;

    vector_initializer<0, size>::init(out, first, args...);

    return out;
  }
};

//================
// }}} Constructor

// {{{ Swizzle
//============

template<size_t dst_index, size_t index, size_t... other_indices>
struct swizzle_initializer final
{
  template<typename vector_type>
  static void init(const vector_type& in, vector_type& out) noexcept
  {
    out.template at<dst_index>() = in.template at<index>();

    swizzle_initializer<dst_index + 1, other_indices...>::init(in, out);
  }
};

template<size_t dst_index, size_t last_index>
struct swizzle_initializer<dst_index, last_index> final
{
  template<typename vector_type>
  static void init(const vector_type& in, vector_type& out) noexcept
  {
    out.template at<dst_index>() = in.template at<last_index>();
  }
};

template<size_t... indices>
struct swizzle final
{
  template<typename scalar, size_t vec_size>
  static auto get(const vector<scalar, vec_size>& v) noexcept
    -> vector<scalar, sizeof...(indices)>
  {
    vector<scalar, sizeof...(indices)> out;

    swizzle_initializer<0, indices...>::init(v, out);

    return out;
  }
};

template<size_t single_index>
struct swizzle<single_index> final
{
  template<typename scalar, size_t vec_size>
  static auto get(const vector<scalar, vec_size>& v) noexcept -> scalar
  {
    return v.template at<single_index>();
  }
};

//============
// }}} Swizzle

//===========
// }}} Vector

// {{{ Matrix
//===========

template<typename scalar, size_t row_count, size_t column_count>
class matrix final
{
public:
  using column_type = vector<scalar, row_count>;

  template<size_t index>
  column_type& at() noexcept
  {
    static_assert(index < column_count, "Column index is out of bounds.");
    return columns[index];
  }

  template<size_t index>
  const column_type& at() const noexcept
  {
    static_assert(index < column_count, "Column index is out of bounds.");
    return columns[index];
  }

private:
  column_type columns[column_count];
};

template<typename scalar>
using mat2 = matrix<scalar, 2, 2>;

template<typename scalar>
using mat3 = matrix<scalar, 3, 3>;

template<typename scalar>
using mat4 = matrix<scalar, 4, 4>;

template<typename scalar, size_t row_count, size_t column_count>
auto
operator+(const matrix<scalar, row_count, column_count>& a,
          const matrix<scalar, row_count, column_count>& b) noexcept
  -> matrix<scalar, row_count, column_count>
{
  matrix<scalar, row_count, column_count> out;

  binary_op<column_count>::add(a, b, out);

  return out;
}

template<typename scalar, size_t row_count, size_t column_count>
auto
operator-(const matrix<scalar, row_count, column_count>& a,
          const matrix<scalar, row_count, column_count>& b) noexcept
  -> matrix<scalar, row_count, column_count>
{
  matrix<scalar, row_count, column_count> out;

  binary_op<column_count>::sub(a, b, out);

  return out;
}

template<size_t row, size_t column, size_t row_count, size_t column_count>
struct diagnonal_matrix_initializer final
{
  template<typename scalar>
  using matrix_type = matrix<scalar, row_count, column_count>;

  template<typename scalar>
  static void init(matrix_type<scalar>& m, scalar n) noexcept
  {
    auto init_value = (row == column) ? n : scalar(0);

    m.template at<column>().template at<row>() = init_value;

    diagnonal_matrix_initializer<row + 1, column, row_count, column_count>::
      init(m, n);
  }
};

template<size_t column, size_t row_count, size_t column_count>
struct diagnonal_matrix_initializer<row_count, column, row_count, column_count>
  final
{
  template<typename scalar>
  using matrix_type = matrix<scalar, row_count, column_count>;

  template<typename scalar>
  static void init(matrix_type<scalar>& m, scalar n) noexcept
  {
    diagnonal_matrix_initializer<0, column + 1, row_count, column_count>::init(
      m, n);
  }
};

template<size_t row_count, size_t column_count>
struct diagnonal_matrix_initializer<0, column_count, row_count, column_count>
  final
{
  template<typename scalar>
  static void init(matrix<scalar, row_count, column_count>&, scalar) noexcept
  {}
};

template<size_t row_count, size_t column_count>
struct matrix_constructor final
{
  template<typename scalar>
  static auto make(scalar n) noexcept -> matrix<scalar, row_count, column_count>
  {
    matrix<scalar, row_count, column_count> out;

    diagnonal_matrix_initializer<0, 0, row_count, column_count>::init(out, n);

    return out;
  }
};

//===========
// }}} Matrix

// {{{ Frame
//==========

template<typename uniform_data, typename varying_data, typename float_type>
class frame final
{
public:
  void encode_rgb(unsigned char* rgb_buffer) const noexcept
  {
    constexpr float_type min_val(0);
    constexpr float_type max_val(255);

    for (size_t i = 0; i < (m_width * m_height); i++) {

      auto color = m_pixels[i](get_uniform_data());

      auto dst = rgb_buffer + (i * 3);

      dst[0] = clamp(color.template at<0>() * 255, min_val, max_val);
      dst[1] = clamp(color.template at<1>() * 255, min_val, max_val);
      dst[2] = clamp(color.template at<2>() * 255, min_val, max_val);
    }
  }

  const uniform_data& get_uniform_data() const noexcept
  {
    return m_uniform_data;
  }

  uniform_data& get_uniform_data() noexcept { return m_uniform_data; }

  void resize(size_t w, size_t h)
  {
    m_pixels.resize(w * h);
    m_width = w;
    m_height = h;
  }

  void sample_pixels()
  {
    const auto& u_dat = static_cast<const uniform_data&>(m_uniform_data);

    for (size_t y = 0; y < m_height; y++) {

      for (size_t x = 0; x < m_width; x++) {

        float_type u_min = (x + float_type(0)) / float_type(m_width);
        float_type u_max = (x + float_type(1)) / float_type(m_width);

        float_type v_min = (y + float_type(0)) / float_type(m_height);
        float_type v_max = (y + float_type(1)) / float_type(m_height);

        auto uv_min = make_vec2(u_min, v_min);
        auto uv_max = make_vec2(u_max, v_max);

        m_pixels[(y * m_width) + x](u_dat, uv_min, uv_max);
      }
    }
  }

private:
  uniform_data m_uniform_data;
  std::vector<varying_data> m_pixels;
  size_t m_width = 0;
  size_t m_height = 0;
};

//==========
// }}} Frame

//==================
// }}} Builtin Types

} // namespace pathway

// vim: foldmethod=marker

#endif // PATHWAY_COMMON_RUNTIME_H_INCLUDED
