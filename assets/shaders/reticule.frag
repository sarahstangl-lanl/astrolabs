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

/**
 * Uniforms
 */


uniform vec4 C_border = vec4(0.4f, 1.0f, 0.4f, 1.0f);
uniform vec4 C_crosshair = vec4(1.0f, 1.0f, 0.4f, 1.0f);
uniform vec4 C_dot = vec4(1.0f, 0.0f, 0.4f, 1.0f);

void main() {

  float border = 0.05f;

  float crosshair_thickness = 0.02f;
  float crosshair_threshold = 0.1f;

  float border_threshold = 0.5f - border;

  // Size of the central dot
  float R_dot = 0.02f;

//  color_out_ = color_;
//  color_out_.rg = tex_;

  float dx = abs(tex_.x - 0.5);
  float dy = abs(tex_.y - 0.5);

  color_out_ = vec4(0.0);

  if((dx > border_threshold) || (dy > border_threshold)){
    color_out_ = C_border;
  }else if((dx < crosshair_thickness ) && (dy > crosshair_threshold)){
    color_out_ = C_crosshair;
  }else if((dy < crosshair_thickness ) && (dx > crosshair_threshold)){
    color_out_ = C_crosshair;
  }else if( (dx*dx + dy*dy) < R_dot * R_dot){
    color_out_ = C_dot;
  }else{
      discard;
  }

}
