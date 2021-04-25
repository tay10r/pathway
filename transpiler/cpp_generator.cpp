#include "cpp_generator.h"

namespace {

const char* standard_impl = R"(
namespace {

template <typename T>
constexpr T
Clamp(T x, T min, T max) noexcept
{
  x = (x < max) ? x : max;
  x = (x > min) ? x : min;
  return x;
}

void
SamplePixel(const Frame& frame, Pixel& pixel, vec2 uv) noexcept;

vec3
ExportPixel(const Frame& frame, const Pixel& pixel) noexcept;

} // namespace

void
Frame::Resize(std::size_t w, std::size_t h)
{
  mPixels.resize(w * h);
  mWidth = w;
  mHeight = h;
}

void
Frame::Export(float* rgbBuffer) const noexcept
{
  for (std::size_t y = 0; y < mHeight; y++) {

    for (std::size_t x = 0; x < mWidth; x++) {

      const auto& pixel = mPixels[(y * mWidth) + x];

      vec3 color = ExportPixel(*this, pixel);

      rgbBuffer[0] = Clamp(color.x * 255.0f, 0.0f, 255.0f);
      rgbBuffer[1] = Clamp(color.y * 255.0f, 0.0f, 255.0f);
      rgbBuffer[2] = Clamp(color.z * 255.0f, 0.0f, 255.0f);

      rgbBuffer += 3;
    }
  }
}

void
Frame::Sample() noexcept
{
  for (std::size_t y = 0; y < mHeight; y++) {

    for (std::size_t x = 0; x < mWidth; x++) {

      vec2 uv {
        (x + 0.5f) / mWidth,
        (y + 0.5f) / mHeight
      };

      Pixel& pixel = mPixels[(y * mWidth) + x];

      SamplePixel(*this, pixel, uv);
    }
  }
}

template <std::size_t index>
struct VectorMember final { };

template <>
struct VectorMember<std::size_t(0)> final
{
  template <typename vector_type>
  static constexpr auto get(vector_type v) noexcept
  {
    return v.x;
  }
};

template <>
struct VectorMember<std::size_t(1)> final
{
  template <typename vector_type>
  static constexpr auto get(vector_type v) noexcept
  {
    return v.y;
  }
};

template <>
struct VectorMember<std::size_t(2)> final
{
  template <typename vector_type>
  static constexpr auto get(vector_type v) noexcept
  {
    return v.z;
  }
};

template <>
struct VectorMember<std::size_t(3)> final
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
struct Swizzle4 final
{
  template <typename src_type>
  static constexpr dst_type get(src_type in) noexcept
  {
    return {
      VectorMember<a>::get(in),
      VectorMember<b>::get(in),
      VectorMember<c>::get(in),
      VectorMember<d>::get(in)
    };
  }
};

template <typename dst_type,
          std::size_t a,
          std::size_t b,
          std::size_t c>
struct Swizzle3 final
{
  template <typename src_type>
  static constexpr dst_type get(src_type in) noexcept
  {
    return {
      VectorMember<a>::get(in),
      VectorMember<b>::get(in),
      VectorMember<c>::get(in)
    };
  }
};

template <typename dst_type,
          std::size_t a,
          std::size_t b>
struct Swizzle2 final
{
  template <typename src_type>
  static constexpr dst_type get(src_type in) noexcept
  {
    return {
      VectorMember<a>::get(in),
      VectorMember<b>::get(in)
    };
  }
};

template <typename dst_type, std::size_t a>
struct Swizzle1 final
{
  template <typename src_type>
  static constexpr dst_type get(src_type in) noexcept
  {
    return VectorMember<a>::get(in);
  }
};
)";

} // namespace

void
cpp_generator::GenerateIntVectorSwizzle(const member_expr& e)
{
  swizzle s(*e.member_name.identifier);

  switch (s.member_indices.size()) {
    case 1:
      this->os << "Swizzle1<int, ";
      this->os << s.member_indices[0];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 2:
      this->os << "Swizzle2<vec2i, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 3:
      this->os << "Swizzle3<vec3i, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ", " << s.member_indices[2];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 4:
      this->os << "Swizzle4<vec4i, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ", " << s.member_indices[2] << ", " << s.member_indices[3];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
  }
}

void
cpp_generator::GenerateFloatVectorSwizzle(const member_expr& e)
{
  swizzle s(*e.member_name.identifier);

  switch (s.member_indices.size()) {
    case 1:
      this->os << "Swizzle1<float, ";
      this->os << s.member_indices[0];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 2:
      this->os << "Swizzle2<vec2, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 3:
      this->os << "Swizzle3<vec3, ";
      this->os << s.member_indices[0] << ", " << s.member_indices[1];
      this->os << ", " << s.member_indices[2];
      this->os << ">::get(";
      e.base_expr->accept(*this);
      this->os << ")";
      break;
    case 4:
      this->os << "Swizzle4<vec4, ";
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
    case TypeID::Void:
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
      GenerateFloatVectorSwizzle(e);
      break;
    case TypeID::Vec2i:
    case TypeID::Vec3i:
    case TypeID::Vec4i:
      GenerateIntVectorSwizzle(e);
      break;
  }
}

void
cpp_generator::generate_pixel_state(const Program& program)
{
  this->os << "struct Pixel final" << std::endl;

  this->os << "{" << std::endl;

  this->increase_indent();

  for (const auto* varyingVar : program.VaryingGlobalVars()) {

    this->indent() << to_string(varyingVar->GetTypeID()) << " "
                   << varyingVar->Identifier() << ";" << std::endl;
  }

  this->decrease_indent();

  this->os << "};" << std::endl;
}

void
cpp_generator::generate_frame_state(const Program& program)
{
  this->os << "struct Frame final" << std::endl;

  this->os << "{" << std::endl;

  this->increase_indent();

  for (const auto* uniformVar : program.UniformGlobalVars()) {

    this->indent() << to_string(uniformVar->GetTypeID()) << " "
                   << uniformVar->Identifier();

    if (uniformVar->init_expr) {

      this->os << " = ";

      uniformVar->init_expr->accept(*this);
    }

    this->os << ";" << std::endl;
  }

  this->indent() << "void Export(float* rgbBuffer) const noexcept;"
                 << std::endl;

  this->indent() << "void Sample() noexcept;" << std::endl;
  this->indent() << "void Resize(std::size_t w, std::size_t h);" << std::endl;

  this->decrease_indent();

  this->indent() << "private:" << std::endl;

  this->increase_indent();

  this->indent() << "std::vector<Pixel> mPixels;" << std::endl;

  this->indent() << "std::size_t mWidth = 0;" << std::endl;

  this->indent() << "std::size_t mHeight = 0;" << std::endl;

  this->decrease_indent();

  this->os << "};" << std::endl;
}

void
cpp_generator::generate_builtin_types(const Program& program)
{
  this->os << "template <typename scalar>" << std::endl;
  this->os << "struct basic_vec2 final { scalar x, y; };" << std::endl;

  this->blank();

  this->os << "template <typename scalar>" << std::endl;
  this->os << "struct basic_vec3 final { scalar x, y, z; };" << std::endl;

  this->blank();

  this->os << "template <typename scalar>" << std::endl;
  this->os << "struct alignas(16) basic_vec4 final { scalar x, y, z, w; };"
           << std::endl;

  this->blank();

  this->os << "using vec2 = basic_vec2<float>;" << std::endl;
  this->os << "using vec3 = basic_vec3<float>;" << std::endl;
  this->os << "using vec4 = basic_vec4<float>;" << std::endl;

  this->blank();

  this->os << "using vec2i = basic_vec2<std::int32_t>;" << std::endl;
  this->os << "using vec3i = basic_vec3<std::int32_t>;" << std::endl;
  this->os << "using vec4i = basic_vec4<std::int32_t>;" << std::endl;

  this->blank();

  this->os << "struct alignas(16) mat4 final { float data[16]; };" << std::endl;

  this->blank();

  this->generate_pixel_state(program);

  this->blank();

  this->generate_frame_state(program);

  this->blank();

  this->os << "void" << std::endl;
  this->os << "SampleFrame(Frame& frame) noexcept;" << std::endl;

  this->blank();

  this->os << "void" << std::endl;
  this->os << "ExportFrame(const Frame& frame, float* rgbBuffer) noexcept;"
           << std::endl;

  this->blank();
}

void
cpp_generator::generate(const Program& prg)
{
  this->os << "#ifndef PT_H_INCLUDED" << std::endl;
  this->os << "#define PT_H_INCLUDED" << std::endl;

  this->blank();

  this->os << "#include <vector>" << std::endl;

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

  this->os << "void" << std::endl;
  this->os << "SamplePixel(Frame& frame, Pixel& pixel, vec2 uv) noexcept;"
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
