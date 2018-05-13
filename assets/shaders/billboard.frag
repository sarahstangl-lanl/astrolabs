#version 330

uniform sampler2D color_sampler_;

/**
 * Inputs from vertex shader
 */
in vec2 coords_;
in vec4 color_ex_;
in vec3 light_;

out vec4 color_out_;

void main(){
  color_out_ = vec4(0);
  color_out_ = texture(color_sampler_, coords_);
//  color_out_ = vec4(1.0, 0.0, 0.0, 1.0f);
}

