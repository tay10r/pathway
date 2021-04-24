#define PT_IMPLEMENTATION

#include "rtweekend.h"

#include <fstream>
#include <vector>

int
main()
{
  auto fb = pt::create_framebuf();

  pt::resize_framebuf(*fb, 640, 480);

  pt::program prg;

  pt::run(prg, *fb);

  std::vector<unsigned char> colorbuf(640 * 480 * 3);

  pt::encode_framebuf(*fb, colorbuf.data(), 0, 0, 640, 480);

  pt::destroy_framebuf(fb);

  std::ofstream file("rtweekend.ppm");

  file << "P6\n640 480\n255\n";

  file.write((const char*)colorbuf.data(), colorbuf.size());

  return 0;
}
