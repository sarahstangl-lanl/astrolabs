#version 330

in vec4 color_ex_;
in vec4 color_vert_;

out vec4 color_out_;

void main(){
  color_out_ = color_vert_;
}