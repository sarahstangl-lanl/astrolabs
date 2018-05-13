#version 330

uniform sampler2D color_sampler_;

/**
 * Inputs from vertex shader
 */
in vec2 coord_;
in vec4 color_ex_;
in vec3 light_;

out vec4 color_out_;

float aastep (float threshold , float value) {
  float w =  length ( vec2(dFdx(value), dFdy(value)));
  // GLSL 's fwidth(value) is abs(dFdx(value)) + abs(dFdy(value))
  return smoothstep(threshold - w, threshold + w, value);
}

void main(){
  color_out_ = color_ex_;

  float r = length(coord_);

#if 0
  if(r > 1.0){
    discard;
  }
#else

  // Discard some of the fragment but not too
  // close to the edge or we get jaggies!
  if(r > 1.1){
    discard;
  }
  color_out_.a = 1.0f - aastep(1.0, r);
#endif

}

