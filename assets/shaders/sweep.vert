#version 330
uniform mat4 p_;
uniform mat4 mv_;
uniform vec4 color_ = vec4(1.0, 1.0, 1.0, 1.0);
uniform float z_offset_ = 0.0;

in vec3 position;
out vec4 color_ex;

void main() {
   color_ex = color_;
   gl_Position = p_ * mv_ * vec4(position.x, position.y, position.z + z_offset_, 1.0);
}
