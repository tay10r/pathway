Pathway
=======

Pathway is a (work-in-progress) programming language for path tracing.

There's several features planned for it:

 - [x] Translation to C++
 - [ ] Translation to CUDA
 - [ ] Translation to GLSL
 - [ ] Translation to ISPC
 - [ ] Builtin support for integration
 - [ ] Builtin support for traversing BVHs
 - [ ] Reproducible random number generation
 - [ ] Automatic stratification of sampling

The syntax is based on GLSL. Here's an example.

```glsl
/* Radiance can be modeled as simple RGB or something more complicated,
 * by just changing the global declarations in the shader.
 */

varying vec3 g_pixel_color = vec3(0.0, 0.0, 0.0);

/* The function below is called whenever a sample is to be taken
 * of the pixel. It may be called several times or just once per
 * frame, depending on the code that dispatches the shader. It is
 * passed the minimum and maximum UV coordinates of the pixel.
 */

void sample_pixel(vec2 uv_min, vec2 uv_max) {
  g_pixel_color = (uv_min + uv_max) * 0.5;
}

/* Before a frame can be displayed in a window or saved into an
 * image file, there has to be a conversion from the shader-specific
 * radiance model to RGBA values. The job of this function is to do
 * just that.
 */

vec4 encode_pixel() {
  return vec4(g_pixel_color.xyz, 1.0);
}
```

While some basic programs like the one above work just fine, there's still a lot to be done
before it is considered usable. Feel free to open an issue if there's any questions.
