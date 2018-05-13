#version 330

/**
 *
 *
 */

// Can probably get rid of this matrix
uniform mat4 mvp_;
uniform vec4 color_ = vec4(1.0, 1.0, 0.0, 0.5);

/**
 *
 * Input Attributes
 *
 */

// The vertex positions
in vec3 position_in_;

// Texture coordinates into the atlas.
in vec2 tex_in_;

/**
 *
 * Outputs to fragment Shader
 *
 */
out vec4 color_ex;
out vec2 tex_;

void main() {
  color_ex = color_;
  tex_ = tex_in_;

  gl_Position = mvp_ * vec4(position_in_, 1.0);
}