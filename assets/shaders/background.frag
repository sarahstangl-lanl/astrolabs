#version 330

/**
 Draw the full-screen background by interpolating a cube of backgrounds
 */

// Background interpolator
// has multiple slices of the background image and we ineterpolate through it
uniform sampler3D background_;

// The foreground is then 1.0 - background_alpha;
uniform float background_time_ = 1.0;

// The foreground is then 1.0 - background_alpha;
uniform float background_alpha_ = 1.0;

in vec2 coords_;
out vec4 color_out;

void main() {
  color_out = texture(background_, vec3(coords_, background_time_));
  color_out.a = background_alpha_ * color_out.a;

//  color_out.rga = vec3(0.0, 1.0, background_alpha_);
}
