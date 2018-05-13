#version 330

const float M_PI = 3.14159265358979323846;


/**
 * Draw a full-screen triangle or whatever.
 */

//layout (location = 0) in vec3 position;
in vec3 position_in_;

// The scale of the background
uniform vec3 scale_ = vec3(1.0);

uniform mat4 mvp_;

out vec4 color_ex_;
out vec2 coords_;

void main(void){
  color_ex_ = vec4(1.0, 1.0, 0.0, 1.0);
  coords_ = position_in_.xy * 0.5 + 0.5;
  coords_.y = 1.0f - coords_.y;

  vec3 rotated_position = vec3(position_in_);

  rotated_position.z = position_in_.y;
  rotated_position.y = - position_in_.z;

  gl_Position = mvp_ * vec4(scale_ * rotated_position, 1.0);
}
