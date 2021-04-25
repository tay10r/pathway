#define PT_IMPLEMENTATION

#include "rtweekend.h"

#include <fstream>
#include <vector>

int
main()
{
  pt::Frame frame;

  frame.Resize(640, 480);

  frame.Sample();

  std::vector<unsigned char> colorBuffer(640 * 480 * 3);

  frame.EncodeRGB(colorBuffer.data());

  std::ofstream file("rtweekend.ppm");

  file << "P6\n640 480\n255\n";

  file.write((const char*)colorBuffer.data(), colorBuffer.size());

  return 0;
}
