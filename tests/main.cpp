#include <pathway.h>

#include HEADER

#include <limits>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION

#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "stb_image_write.h"

#include <fstream>
#include <iostream>

namespace {

const std::size_t width = 320;

const std::size_t height = 240;

bool
FileExists(const char* path)
{
  std::ifstream file(path);
  return file.good();
}

using byte = unsigned char;

byte*
LoadImage(const char* path)
{
  FILE* file = fopen(path, "rb");

  assert(file);

  int w = 0;
  int h = 0;
  byte* colorBuffer = stbi_load_from_file(file, &w, &h, nullptr, 3);

  assert(colorBuffer);
  assert(w == width);
  assert(h == height);

  fclose(file);

  return colorBuffer;
}

std::vector<byte>
GetDifference()
{
  byte* goodImage = LoadImage(GOOD_IMAGE_PATH);

  byte* testImage = LoadImage(TEST_IMAGE_PATH);

  std::vector<byte> diff;

  if (memcmp(goodImage, testImage, width * height * 3) == 0)
    return diff;

  float minError = std::numeric_limits<float>::infinity();

  float maxError = -std::numeric_limits<float>::infinity();

  for (std::size_t i = 0; i < (width * height * 3); i++) {
    minError = std::min(minError, float(goodImage[i]) - testImage[i]);
    maxError = std::max(maxError, float(goodImage[i]) - testImage[i]);
  }

  float errorRange = maxError - minError;

  diff.resize(width * height * 3);

  for (std::size_t i = 0; i < (width * height * 3); i++) {

    float error = float(goodImage[i]) - testImage[i];

    float normalizedError = (error - minError) / errorRange;

    diff[i] = std::min(255.0f, normalizedError * 255.0f);
  }

  free(goodImage);
  free(testImage);

  return diff;
}

} // namespace

int
main()
{
  using uniform_data = example::uniform_data<float, int>;

  using varying_data = example::varying_data<float, int>;

  pathway::frame<uniform_data, varying_data, float> frame;

  frame.resize(width, height);

  frame.sample_pixels();

  std::vector<unsigned char> rgbBuffer(width * height * 3);

  frame.encode_rgb(rgbBuffer.data());

  stbi_write_png(
    TEST_IMAGE_PATH, width, height, 3, rgbBuffer.data(), width * 3);

  if (!FileExists(GOOD_IMAGE_PATH)) {
    std::cerr << "Aborting test because known good image does not exist."
              << std::endl;
    return EXIT_FAILURE;
  }

  auto diff = GetDifference();

  if (diff.empty())
    return EXIT_SUCCESS;

  std::cerr << "Failing test because images are different." << std::endl;

  stbi_write_png(DIFF_IMAGE_PATH, width, height, 3, diff.data(), width * 3);

  return EXIT_FAILURE;
}
