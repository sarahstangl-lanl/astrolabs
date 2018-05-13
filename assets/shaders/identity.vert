#version 330

/**
 * Draw a full-screen triangle or whatever.
 */

//layout (location = 0) in vec3 position;
in vec3 position_in_;


// The scale of the background
uniform vec3 scale_ = vec3(1.0);

out vec4 color_ex_;
out vec2 coords_;

void main(void){
  color_ex_ = vec4(1.0, 1.0, 0.0, 1.0);
  coords_ = position_in_.xy * 0.5 + 0.5;
  coords_.y = 1.0f - coords_.y;
  gl_Position = vec4(scale_ * position_in_, 1.0);
}
