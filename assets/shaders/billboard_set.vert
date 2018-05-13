#version 330

/**
 *
 *
 */

// Can probably get rid of this matrix
uniform mat4 mvp_;
uniform mat4 inv_mv_;

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

// The vertex positions
in vec4 color_in_;

/**
 *
 * Outputs to fragment Shader
 *
 */

// Solid color for the entire set
out vec4 color_ex_;

// Color for the given vertex
out vec4 color_vert_;

// Texture coordinate
out vec2 tex_;


void main() {
  color_vert_ = color_in_;
  color_ex_ = color_;
  tex_.xy = tex_in_.xy;
  gl_Position = mvp_ * vec4(position_in_, 1.0);
}