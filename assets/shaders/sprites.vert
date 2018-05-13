#version 330

uniform mat4 mvp_;

/**
 * Vertex Attributes
 */
in vec3 position_in_;
in float size_in_;
in vec4 color_in_;

out vec4 color_ex;

/**
 * Uniforms
 */

// Fixed offset for entire set of sprites
uniform vec3 offset_ = vec3(0.0);


void main() {
   color_ex =  color_in_;
   gl_PointSize = size_in_;
   gl_Position = mvp_ * vec4(position_in_ + offset_, 1.0);
}
