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

vec4
EncodePixel(const Frame& frame, const Pixel& pixel) noexcept;

} // namespace

void
Frame::Resize(std::size_t w, std::size_t h)
{
  mPixels.resize(w * h);
  mWidth = w;
  mHeight = h;
}

void
Frame::EncodeRGBA(float* rgbaBuffer) const noexcept
{
  for (std::size_t y = 0; y < mHeight; y++) {

    for (std::size_t x = 0; x < mWidth; x++) {

      const auto& pixel = mPixels[(y * mWidth) + x];

      vec4 color = EncodePixel(*this, pixel);

      rgbaBuffer[0] = Clamp(color.x, 0.0f, 1.0f);
      rgbaBuffer[1] = Clamp(color.y, 0.0f, 1.0f);
      rgbaBuffer[2] = Clamp(color.z, 0.0f, 1.0f);
      rgbaBuffer[3] = Clamp(color.w, 0.0f, 1.0f);

      rgbaBuffer += 4;
    }
  }
}

void
Frame::EncodeRGB(unsigned char* rgbBuffer) const noexcept
{
  for (std::size_t y = 0; y < mHeight; y++) {

    for (std::size_t x = 0; x < mWidth; x++) {

      const auto& pixel = mPixels[(y * mWidth) + x];

      vec4 color = EncodePixel(*this, pixel);

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

template <typename DstType,
          std::size_t a,
          std::size_t b,
          std::size_t c,
          std::size_t d>
struct Swizzle4 final
{
  template <typename SrcType>
  static constexpr DstType get(SrcType in) noexcept
  {
    return {
      VectorMember<a>::get(in),
      VectorMember<b>::get(in),
      VectorMember<c>::get(in),
      VectorMember<d>::get(in)
    };
  }
};

template <typename DstType,
          std::size_t a,
          std::size_t b,
          std::size_t c>
struct Swizzle3 final
{
  template <typename SrcType>
  static constexpr DstType get(SrcType in) noexcept
  {
    return {
      VectorMember<a>::get(in),
      VectorMember<b>::get(in),
      VectorMember<c>::get(in)
    };
  }
};

template <typename DstType,
          std::size_t a,
          std::size_t b>
struct Swizzle2 final
{
  template <typename SrcType>
  static constexpr DstType get(SrcType in) noexcept
  {
    return {
      VectorMember<a>::get(in),
      VectorMember<b>::get(in)
    };
  }
};

template <typename DstType, std::size_t a>
struct Swizzle1 final
{
  template <typename SrcType>
  static constexpr DstType get(SrcType in) noexcept
  {
    return VectorMember<a>::get(in);
  }
};

//====================//
// BINARY OPERATOR: + //
//====================//

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator+(Scalar a, generic_vec2<Scalar> b) noexcept
{
  return generic_vec2<Scalar>(a.x + b.x, a.y + b.y);
}

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator+(generic_vec2<Scalar> a, Scalar b) noexcept
{
  return b + a;
}

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator+(generic_vec2<Scalar> a, generic_vec2<Scalar> b) noexcept
{
  return { a.x + b.x, a.y + b.y };
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator+(generic_vec3<Scalar> a, Scalar b) noexcept
{
  return { a.x + b, a.y + b, a.z + b };
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator+(Scalar a, generic_vec3<Scalar> b) noexcept
{
  return operator+(b, a);
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator+(generic_vec3<Scalar> a, generic_vec3<Scalar> b) noexcept
{
  return { a.x + b.x, a.y + b.y, a.z + b.z };
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator+(generic_vec4<Scalar> a, Scalar b) noexcept
{
  return { a.x + b, a.y + b, a.z + b, a.w + b };
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator+(Scalar a, generic_vec4<Scalar> b) noexcept
{
  return operator+(b, a);
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator+(generic_vec4<Scalar> a, generic_vec4<Scalar> b) noexcept
{
  return { a.x + b.x, a.y + b.y, a.z + b.z, a.w + b.w };
}

//====================//
// BINARY OPERATOR: - //
//====================//

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator-(Scalar a, generic_vec2<Scalar> b) noexcept
{
  return generic_vec2<Scalar>(a.x - b.x, a.y - b.y);
}

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator-(generic_vec2<Scalar> a, Scalar b) noexcept
{
  return b - a;
}

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator-(generic_vec2<Scalar> a, generic_vec2<Scalar> b) noexcept
{
  return { a.x - b.x, a.y - b.y };
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator-(generic_vec3<Scalar> a, Scalar b) noexcept
{
  return { a.x - b, a.y - b, a.z - b };
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator-(Scalar a, generic_vec3<Scalar> b) noexcept
{
  return b - a;
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator-(generic_vec3<Scalar> a, generic_vec3<Scalar> b) noexcept
{
  return { a.x - b.x, a.y - b.y, a.z - b.z };
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator-(generic_vec4<Scalar> a, Scalar b) noexcept
{
  return { a.x - b, a.y - b, a.z - b, a.w - b };
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator-(Scalar a, generic_vec4<Scalar> b) noexcept
{
  return operator-(b, a);
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator-(generic_vec4<Scalar> a, generic_vec4<Scalar> b) noexcept
{
  return { a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w };
}

//====================//
// BINARY OPERATOR: * //
//====================//

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator*(Scalar a, generic_vec2<Scalar> b) noexcept
{
  return generic_vec2<Scalar>(a.x * b.x, a.y * b.y);
}

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator*(generic_vec2<Scalar> a, Scalar b) noexcept
{
  return b * a;
}

template <typename Scalar>
constexpr generic_vec2<Scalar>
operator*(generic_vec2<Scalar> a, generic_vec2<Scalar> b) noexcept
{
  return { a.x * b.x, a.y * b.y };
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator*(generic_vec3<Scalar> a, Scalar b) noexcept
{
  return { a.x * b, a.y * b, a.z * b };
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator*(Scalar a, generic_vec3<Scalar> b) noexcept
{
  return b * a;
}

template <typename Scalar>
constexpr generic_vec3<Scalar>
operator*(generic_vec3<Scalar> a, generic_vec3<Scalar> b) noexcept
{
  return { a.x * b.x, a.y * b.y, a.z * b.z };
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator*(generic_vec4<Scalar> a, Scalar b) noexcept
{
  return { a.x * b, a.y * b, a.z * b, a.w * b };
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator*(Scalar a, generic_vec4<Scalar> b) noexcept
{
  return operator*(b, a);
}

template <typename Scalar>
constexpr generic_vec4<Scalar>
operator*(generic_vec4<Scalar> a, generic_vec4<Scalar> b) noexcept
{
  return { a.x * b.x, a.y * b.y, a.z * b.z, a.w * b.w };
}
)";

} // namespace

void
CPPGenerator::GenerateIntVectorSwizzle(const MemberExpr& e, size_t vecSize)
{
  auto swizzleResult = Swizzle::Make(e.MemberName().Identifier(), vecSize);

  if (!swizzleResult)
    return;

  const auto& swizzle = swizzleResult.value();

  switch (swizzle.Size()) {
    case 1:
      os << "Swizzle1<int, ";
      os << swizzle.At(0);
      os << ">::get(";
      e.BaseExpr().AcceptVisitor(*this);
      os << ")";
      break;
    case 2:
      os << "Swizzle2<vec2i, ";
      os << swizzle.At(0) << ", " << swizzle.At(1);
      os << ">::get(";
      e.BaseExpr().AcceptVisitor(*this);
      os << ")";
      break;
    case 3:
      os << "Swizzle3<vec3i, ";
      os << swizzle.At(0) << ", " << swizzle.At(1);
      os << ", " << swizzle.At(2);
      os << ">::get(";
      e.BaseExpr().AcceptVisitor(*this);
      os << ")";
      break;
    case 4:
      os << "Swizzle4<vec4i, ";
      os << swizzle.At(0) << ", " << swizzle.At(1);
      os << ", " << swizzle.At(2) << ", " << swizzle.At(3);
      os << ">::get(";
      e.BaseExpr().AcceptVisitor(*this);
      os << ")";
      break;
  }
}

void
CPPGenerator::GenerateFloatVectorSwizzle(const MemberExpr& e, size_t vecSize)
{
  auto swizzleResult = Swizzle::Make(e.MemberName().Identifier(), vecSize);

  if (!swizzleResult)
    return;

  const auto& swizzle = swizzleResult.value();

  switch (swizzle.Size()) {
    case 1:
      os << "Swizzle1<float, ";
      os << swizzle.At(0);
      os << ">::get(";
      e.BaseExpr().AcceptVisitor(*this);
      os << ")";
      break;
    case 2:
      os << "Swizzle2<vec2, ";
      os << swizzle.At(0) << ", " << swizzle.At(1);
      os << ">::get(";
      e.BaseExpr().AcceptVisitor(*this);
      os << ")";
      break;
    case 3:
      os << "Swizzle3<vec3, ";
      os << swizzle.At(0) << ", " << swizzle.At(1);
      os << ", " << swizzle.At(2);
      os << ">::get(";
      e.BaseExpr().AcceptVisitor(*this);
      os << ")";
      break;
    case 4:
      os << "Swizzle4<vec4, ";
      os << swizzle.At(0) << ", " << swizzle.At(1);
      os << ", " << swizzle.At(2) << ", " << swizzle.At(3);
      os << ">::get(";
      e.BaseExpr().AcceptVisitor(*this);
      os << ")";
      break;
  }
}

void
CPPGenerator::Visit(const MemberExpr& e)
{
  switch (e.BaseExpr().GetType().value().ID()) {
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
      GenerateFloatVectorSwizzle(e, 2);
      break;
    case TypeID::Vec3:
      GenerateFloatVectorSwizzle(e, 3);
      break;
    case TypeID::Vec4:
      GenerateFloatVectorSwizzle(e, 4);
      break;
    case TypeID::Vec2i:
      GenerateIntVectorSwizzle(e, 2);
      break;
    case TypeID::Vec3i:
      GenerateIntVectorSwizzle(e, 3);
      break;
    case TypeID::Vec4i:
      GenerateIntVectorSwizzle(e, 4);
      break;
  }
}

void
CPPGenerator::generate_pixel_state(const Program& program)
{
  os << "struct Pixel final" << std::endl;

  os << "{" << std::endl;

  IncreaseIndent();

  for (const auto* varyingVar : program.VaryingGlobalVars()) {

    Indent() << ToString(varyingVar->GetTypeID()) << " "
             << varyingVar->Identifier() << ";" << std::endl;
  }

  DecreaseIndent();

  os << "};" << std::endl;
}

void
CPPGenerator::generate_frame_state(const Program& program)
{
  os << "struct Frame final" << std::endl;

  os << "{" << std::endl;

  IncreaseIndent();

  for (const auto* uniformVar : program.UniformGlobalVars()) {

    Indent() << ToString(uniformVar->GetTypeID()) << " "
             << uniformVar->Identifier();

    if (uniformVar->HasInitExpr()) {

      os << " = ";

      uniformVar->InitExpr().AcceptVisitor(*this);
    }

    os << ";" << std::endl;
  }

  Indent() << "void EncodeRGBA(float* rgbaBuffer) const noexcept;" << std::endl;
  Indent() << "void EncodeRGB(unsigned char* rgbBuffer) const noexcept;"
           << std::endl;

  Indent() << "void Sample() noexcept;" << std::endl;
  Indent() << "void Resize(std::size_t w, std::size_t h);" << std::endl;

  DecreaseIndent();

  Indent() << "private:" << std::endl;

  IncreaseIndent();

  Indent() << "std::vector<Pixel> mPixels;" << std::endl;

  Indent() << "std::size_t mWidth = 0;" << std::endl;

  Indent() << "std::size_t mHeight = 0;" << std::endl;

  DecreaseIndent();

  os << "};" << std::endl;
}

void
CPPGenerator::GenerateBinaryExpr(const BinaryExpr& binaryExpr)
{
  binaryExpr.LeftExpr().AcceptVisitor(*this);

  switch (binaryExpr.GetKind()) {
    case BinaryExpr::Kind::Add:
      os << "+";
      break;
    case BinaryExpr::Kind::Sub:
      os << "-";
      break;
    case BinaryExpr::Kind::Mul:
      os << "*";
      break;
    case BinaryExpr::Kind::Div:
      os << "/";
      break;
    case BinaryExpr::Kind::Mod:
      os << "%";
      break;
  }

  binaryExpr.RightExpr().AcceptVisitor(*this);
}

namespace {

const char* gBuiltinTypes = R"(
template <typename scalar>
struct generic_vec2 final
{
  scalar x;
  scalar y;

  constexpr generic_vec2() noexcept = default;

  constexpr generic_vec2(scalar x_, scalar y_) noexcept
    : x(x_)
    , y(y_)
  {}
};

template<typename scalar>
struct generic_vec3 final
{
  scalar x;
  scalar y;
  scalar z;

  constexpr generic_vec3() noexcept = default;

  constexpr generic_vec3(scalar x_, scalar y_, scalar z_) noexcept
    : x(x_)
    , y(y_)
    , z(z_)
  {}

  constexpr generic_vec3(generic_vec2<scalar> xy, scalar z_) noexcept
    : x(xy.x)
    , y(xy.y)
    , z(z_)
  {}

  constexpr generic_vec3(scalar x_, generic_vec2<scalar> yz) noexcept
    : x(x_)
    , y(yz.x)
    , z(yz.y)
  {}
};

template<typename scalar>
struct alignas(16) generic_vec4 final
{
  scalar x;
  scalar y;
  scalar z;
  scalar w;

  constexpr generic_vec4() noexcept = default;

  constexpr generic_vec4(scalar x_, scalar y_, scalar z_, scalar w_) noexcept
    : x(x_)
    , y(y_)
    , z(z_)
    , w(w_)
  {}

  constexpr generic_vec4(generic_vec2<scalar> xy, scalar z_, scalar w_) noexcept
    : x(xy.x)
    , y(xy.y)
    , z(z_)
    , w(w_) {}

  constexpr generic_vec4(scalar x_, generic_vec2<scalar> yz, scalar w_) noexcept
    : x(x_)
    , y(yz.x)
    , z(yz.y)
    , w(w_)
  {}

  constexpr generic_vec4(scalar x_, scalar y_, generic_vec2<scalar> zw) noexcept
    : x(x_)
    , y(y_)
    , z(zw.x)
    , w(zw.y)
  {}

  constexpr generic_vec4(generic_vec2<scalar> xy,
                         generic_vec2<scalar> zw) noexcept
    : x(xy.x)
    , y(xy.y)
    , z(zw.x)
    , w(zw.y)
  {}

  constexpr generic_vec4(generic_vec3<scalar> xyz, scalar w_) noexcept
    : x(xyz.x)
    , y(xyz.y)
    , z(xyz.z)
    , w(w_)
  {}

  constexpr generic_vec4(scalar x_, generic_vec3<scalar> yzw) noexcept
    : x(x_)
    , y(yzw.x)
    , z(yzw.y)
    , w(yzw.z)
  {}
};

using vec2 = generic_vec2<float>;
using vec3 = generic_vec3<float>;
using vec4 = generic_vec4<float>;

using vec2i = generic_vec2<std::int32_t>;
using vec3i = generic_vec3<std::int32_t>;
using vec4i = generic_vec4<std::int32_t>;

struct alignas(16) mat2 final
{
  float data[4];
};

struct mat3 final
{
  float data[9];
};

struct alignas(16) mat4 final
{
  float data[16];
};
)";

} // namespace

void
CPPGenerator::generate_builtin_types(const Program& program)
{
  os << gBuiltinTypes;

  this->generate_pixel_state(program);

  this->Blank();

  this->generate_frame_state(program);
}

void
CPPGenerator::generate(const FuncDecl& fn, const ParamList& params)
{
  std::vector<std::string> paramStringList;

  if (fn.ReferencesGlobalState() || fn.IsEntryPoint()) {

    if (fn.ReferencesFrameState())
      paramStringList.emplace_back("const Frame& frame");
    else if (fn.IsEntryPoint())
      paramStringList.emplace_back("const Frame&");

    if (fn.IsPixelEncoder())
      paramStringList.emplace_back("const Pixel& pixel");
    else if (fn.ReferencesPixelState())
      paramStringList.emplace_back("Pixel& pixel");
    else if (fn.IsPixelSampler())
      paramStringList.emplace_back("Pixel&");
  }

  for (size_t i = 0; i < params.size(); i++) {

    std::ostringstream paramStream;

    paramStream << ToString(params.at(i)->GetType());

    paramStream << ' ';

    paramStream << params.at(i)->Identifier();

    paramStringList.emplace_back(paramStream.str());
  }

  os << '(';

  for (size_t i = 0; i < paramStringList.size(); i++) {

    os << paramStringList[i];

    if ((i + 1) < paramStringList.size())
      os << ", ";
  }

  os << ')';
}

void
CPPGenerator::Generate(const Program& prg)
{
  os << "#ifndef PT_H_INCLUDED" << std::endl;
  os << "#define PT_H_INCLUDED" << std::endl;

  this->Blank();

  os << "#include <vector>" << std::endl;

  this->Blank();

  os << "#include <cstdint>" << std::endl;
  os << "#include <cstddef>" << std::endl;

  this->Blank();

  os << "namespace pt {" << std::endl;

  this->Blank();

  this->generate_builtin_types(prg);

  this->Blank();

  os << "} // namespace pt" << std::endl;

  this->Blank();

  os << "#endif // PT_H_INCLUDED" << std::endl;

  this->Blank();

  os << "#ifdef PT_IMPLEMENTATION" << std::endl;

  this->Blank();

  os << "#include <vector>" << std::endl;

  this->Blank();

  os << "namespace pt {" << std::endl;

  // special case: no Blank line needed here

  os << standard_impl;

  this->Blank();

  os << "namespace {" << std::endl;

  this->Blank();

  this->generate_prototypes(prg);

  this->Blank();

  this->generate_definitions(prg);

  os << "} // namespace" << std::endl;

  this->Blank();

  os << "} // namespace pt" << std::endl;

  this->Blank();

  os << "#endif // PT_IMPLEMENTATION" << std::endl;
}
