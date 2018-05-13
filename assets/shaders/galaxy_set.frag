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

  // This tries to remove backgrounds (map black to alpha = 0.0)
  // TODO: Use the step function properly
  float color_len = length(color_out_.xyz);
  if(step(color_len, 0.1) > 0.00){
    color_out_.a = 0.0f;
    discard;
  }

//  color_out_.rgba = vec4(1.0, 1.0, 0.0, 1.0);

  color_out_.a = color_out_.a * global_alpha_;
}
