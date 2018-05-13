#version 330

uniform vec4 C_base_ = vec4(1.0f, 0.8f, 0.5f, 0.9f);

/**
 *
 * Input from Vertex shader
 *
 */
in vec2 coord_;
in vec4 color_ex_;

out vec4 color_out_;

float aastep (float threshold , float value) {
  float w =  length ( vec2(dFdx(value), dFdy(value)));
  // GLSL 's fwidth(value) is abs(dFdx(value)) + abs(dFdy(value))
  return smoothstep(threshold - w, threshold + w, value);
}

void main() {

  float r = 1.0 - length(coord_);

  color_out_.rgba = r * C_base_;
}
