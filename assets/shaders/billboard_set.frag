#version 330

// Texture atlas
uniform sampler2D atlas_;

// This is for foreground / background blending.
uniform float global_alpha_ = 1.0f;

/**
 *
 * Input from Vertex shader
 *
 */
in vec4 color_ex;
in vec2 tex_;

out vec4 color_out_;


void main() {

  color_out_ = vec4(1.0);
  color_out_ = texture(atlas_, tex_);

//// Background Color
//uniform vec4 C_background_ = vec4(0.0);
//// Text (or mask color)
//uniform vec4 C_text_ = vec4(1.0);
//  color_out_ = mix(C_background_, C_text, texture(atlas_, tex_).a);
  return;
}
