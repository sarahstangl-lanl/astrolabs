#version 330

/**
 *
 * Input from Vertex shader
 *
 */

// Color for the given vertex
in vec4 color_vert_;

// Texture for the quad
in vec2 tex_;

// Output
out vec4 color_out_;

void main() {

  vec4 C_border = color_vert_; // vec4(0.4f, 1.0f, 0.4f, 1.0f);
  float border = 0.05;

//  color_out_ = color_;
//  color_out_.rg = tex_;

  color_out_ = vec4(0.0);

  if((tex_.x < border) || tex_.x > (1.0 - border)){
    color_out_ = C_border;
  }else if((tex_.y < border) || tex_.y > (1.0 - border)){
    color_out_ = C_border;
  }else{
    discard;
  }
}
