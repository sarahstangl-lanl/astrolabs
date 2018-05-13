#version 330

/**
 Draw the full-screen background by interpolating a cube of backgrounds
 */

in vec4 color_ex_;
in vec2 coords_;

out vec4 color_out;

void main() {
  color_out = color_ex_;
  color_out.rg = coords_.xy;
}
