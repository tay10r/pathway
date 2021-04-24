#include "cpp_generator.h"

namespace {

const char* standard_impl = R"(
namespace {

float clamp(float x, float min, float max) noexcept
{
  x = (x < max) ? x : max;
  x = (x > min) ? x : min;
  return x;
}

} // namespace

struct pixel_state final
{
  float r;
  float g;
  float b;
};

struct framebuf final
{
  std::vector<pixel_state> pixels;
  std::size_t w = 0;
  std::size_t h = 0;
};

auto create_framebuf() -> framebuf*
{
  return new framebuf();
}

void destroy_framebuf(framebuf* fb)
{
  delete fb;
}

void resize_framebuf(framebuf& fb, std::size_t w, std::size_t h)
{
  fb.pixels.resize(w * h);
  fb.w = w;
  fb.h = h;
}

void encode_framebuf(const framebuf& fb,
                     unsigned char* rgb_buf,
                     std::size_t x0,
                     std::size_t y0,
                     std::size_t x1,
                     std::size_t y1) noexcept
{
  x0 = (x0 < fb.w) ? x0 : fb.w;
  y0 = (y0 < fb.h) ? y0 : fb.h;

  x1 = (x1 < fb.w) ? x1 : fb.w;
  y1 = (y1 < fb.h) ? y1 : fb.h;

  for (std::size_t y = y0; y < y1; y++) {

    for (std::size_t x = x0; x < x1; x++) {

      const auto& px = fb.pixels[(y * fb.w) + x];

      rgb_buf[0] = clamp(px.r * 255.0f, 0.0f, 255.0f);
      rgb_buf[1] = clamp(px.g * 255.0f, 0.0f, 255.0f);
      rgb_buf[2] = clamp(px.b * 255.0f, 0.0f, 255.0f);

      rgb_buf += 3;
    }
  }
}

void run(const program& prg, framebuf& fb) noexcept
{
  for (std::size_t y = 0; y < fb.h; y++) {

    for (std::size_t x = 0; x < fb.w; x++) {

      vec2 uv {
        (x + 0.5f) / fb.w,
        (y + 0.5f) / fb.h
      };

      vec3 color = shader_main(prg, uv);

      pixel_state& px = fb.pixels[(y * fb.w) + x];

      px.r += color.x;
      px.g += color.y;
      px.b += color.z;
    }
  }
}

template <std::size_t index>
struct vector_member final { };

template <>
struct vector_member<std::size_t(0)> final
{
  template <typename vector_type>
  static constexpr auto get(vector_type v) noexcept
  {
    return v.x;
  }
};

template <>
struct vector_member<std::size_t(1)> final
{
  template <typename vector_type>
  static constexpr auto get(vector_type v) noexcept
  {
    return v.y;
  }
};

template <>
struct vector_member<std::size_t(2)> final
{
  template <typename vector_type>
  static constexpr auto get(vector_type v) noexcept
  {
    return v.z;
  }
};

template <>
struct vector_member<std::size_t(3)> final
{
  template <typename vector_type>
  static constexpr auto get(vector_type v) noexcept
  {
    return v.w;
  }
};

template <typename dst_type,
          std::size_t a,
          std::size_t b,
          std::size_t c,
          std::size_t d>
struct swizzle4 final
{
  template <typename src_type>
  static constexpr dst_type get(src_type in) noexcept
  {
    return {
      vector_member<a>::get(in),
      vector_member<b>::get(in),
      vector_member<c>::get(in),
      vector_member<d>::get(in)
    };
  }
};

template <typename dst_type,
          std::size_t a,
          std::size_t b,
          std::size_t c>
struct swizzle3 final
{
  template <typename src_type>
  static constexpr dst_type get(src_type in) noexcept
  {
    return {
      vector_member<a>::get(in),
      vector_member<b>::get(in),
      vector_member<c>::get(in)
    };
  }
};

template <typename dst_type,
          std::size_t a,
          std::size_t b>
struct swizzle2 final
{
  template <typename src_type>
  static constexpr dst_type get(src_type in) noexcept
  {
    return {
      vector_member<a>::get(in),
      vector_member<b>::get(in)
    };
  }
};

template <typename dst_type, std::size_t a>
struct swizzle1 final
{
  template <typename src_type>
  static constexpr dst_type get(src_type in) noexcept
  {
    return vector_member<a>::get(in);
  }
};
)";

} // namespace

void
cpp_generator::generate_int_vec_swizzle(const member_expr& e)
{
  swizzle s(*e.member_name.identifier);

  switch (s.member_indices.size()) {
    case 1:
      this->os << "swizzle1<int, ";
      this->os << s.member_indices[0];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 2:
      this->os << "swizzle2<vec2i, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 3:
      this->os << "swizzle3<vec3i, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ", " << s.member_indices[2];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 4:
      this->os << "swizzle4<vec4i, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ", " << s.member_indices[2] << ", " << s.member_indices[3];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
  }
}

void
cpp_generator::generate_float_vec_swizzle(const member_expr& e)
{
  swizzle s(*e.member_name.identifier);

  switch (s.member_indices.size()) {
    case 1:
      this->os << "swizzle1<float, ";
      this->os << s.member_indices[0];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 2:
      this->os << "swizzle2<vec2, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 3:
      this->os << "swizzle3<vec3, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ", " << s.member_indices[2];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 4:
      this->os << "swizzle4<vec4, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ", " << s.member_indices[2] << ", " << s.member_indices[3];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
  }
}

void
cpp_generator::visit(const member_expr& e)
{
  switch (e.base_expr->GetType().ID()) {
    case TypeID::Bool:
    case TypeID::Int:
    case TypeID::Float:
    case TypeID::Mat2:
    case TypeID::Mat3:
    case TypeID::Mat4:
      assert(false);
      return;
    case TypeID::Vec2:
    case TypeID::Vec3:
    case TypeID::Vec4:
      generate_float_vec_swizzle(e);
      break;
    case TypeID::Vec2i:
    case TypeID::Vec3i:
    case TypeID::Vec4i:
      generate_int_vec_swizzle(e);
      break;
  }
}

void
cpp_generator::generate(const program& prg)
{
  this->os << "#ifndef PT_H_INCLUDED" << std::endl;
  this->os << "#define PT_H_INCLUDED" << std::endl;

  this->blank();

  this->os << "#include <cstdint>" << std::endl;
  this->os << "#include <cstddef>" << std::endl;

  this->blank();

  this->os << "namespace pt {" << std::endl;

  this->blank();

  this->generate_builtin_types(prg);

  this->blank();

  this->os << "} // namespace pt" << std::endl;

  this->blank();

  this->os << "#endif // PT_H_INCLUDED" << std::endl;

  this->blank();

  this->os << "#ifdef PT_IMPLEMENTATION" << std::endl;

  this->blank();

  this->os << "#include <vector>" << std::endl;

  this->blank();

  this->os << "namespace pt {" << std::endl;

  this->blank();

  this->os << "namespace {" << std::endl;

  this->blank();

  this->os << "vec3 shader_main(const program& prg, vec2 uv) noexcept;"
           << std::endl;

  this->blank();

  this->os << "} // namespace" << std::endl;

  // special case: no blank line needed here

  this->os << standard_impl;

  this->blank();

  this->os << "namespace {" << std::endl;

  this->blank();

  this->generate_prototypes(prg);

  this->blank();

  this->generate_definitions(prg);

  this->os << "} // namespace" << std::endl;

  this->blank();

  this->os << "} // namespace pt" << std::endl;

  this->blank();

  this->os << "#endif // PT_IMPLEMENTATION" << std::endl;
}
