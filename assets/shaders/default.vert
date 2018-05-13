#version 330

// Input
uniform mat4 mvp_;
uniform vec4 solid_color_ = vec4(0);

//  Attributes
in vec3 position_in_;
in vec4 color_in_;

// Output:
out vec4 color_ex_;
out vec4 color_vert_;

void main() {
  color_ex_ = solid_color_;
  color_vert_ = color_in_;
  gl_Position = mvp_ * vec4(position_in_, 1.0);
}