#version 330

uniform mat4 mvp_;
uniform mat4 r_;

uniform vec4 color_ = vec4(1.0, 1.0, 1.0, 0.5);
uniform vec3 offset_ = vec3(0);
uniform vec2 size_ = vec2(6.02, 0.5);
uniform vec2 tex_offset_ = vec2(0.02, 0.0);

in vec4 position;

out vec4 color_ex;

out vec2 tex_;
out vec2 tex_normalized_;

void main() {
  color_ex = color_;
  tex_ = (position.xy + 0.5 * (size_ - tex_offset_));
  tex_normalized_ = (position.xy + 0.5 * size_) / size_;
  gl_Position = mvp_ * (r_ * position + vec4(offset_, 0));
}
