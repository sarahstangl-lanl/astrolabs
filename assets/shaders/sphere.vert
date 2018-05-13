#version 330

// Input
uniform mat4 mvp_;

uniform vec4 solid_color_ = vec4(1.0, 0.0, 0.0, 1.0);
uniform float radius_ = 1.0f;
uniform vec3 offset_ = vec3(0.0);

uniform vec3 camera_up_world_ = vec3(0.0, 1.0, 0.0);
uniform vec3 camera_eye_world_ = vec3(0.0, 0.0, -1.0);
uniform vec3 camera_right_world_ = vec3(1.0, 0.0, 0.0);


/**
 *
 * Vertex Attributes
 *
 */

//  Attributes
in vec3 position_in_;

// Output:
out vec2 coord_;
out vec4 color_ex_;

out vec3 light_;

// The eye
out vec2 eye_relative_;


void main() {
  coord_ = position_in_.xy / radius_;
  color_ex_ = solid_color_;

#if 0
  vec3 world_position = position_in_ + offset_;
#else
  vec3 world_position = offset_ + camera_right_world_ * position_in_.x +  camera_up_world_ * position_in_.y;
  eye_relative_.x = camera_eye_world_.x;
  eye_relative_.y = camera_eye_world_.y;
#endif

  gl_Position = mvp_ * vec4(world_position, 1.0);

  light_ = -world_position;

//  gl_Position = p_ * inv_mv_ * vec4(position_in_ + offset_, 1.0);
}
