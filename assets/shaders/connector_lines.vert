#version 330

// Input
uniform mat4 mvp_;
uniform vec4 solid_color_ = vec4(1.0, 1.0, 0.0f, 1.0f);

//  Attributes
in vec3 position_in_;

// Output:
out vec4 color_ex_;

void main() {
  color_ex_ = solid_color_;
  gl_Position = mvp_ * vec4(position_in_, 1.0);
}