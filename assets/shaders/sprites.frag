#version 330
in vec4 color_ex;
out vec4 color_out_;

float aastep (float threshold , float value) {
  float w =  length ( vec2(dFdx(value), dFdy(value)));
  // GLSL 's fwidth(value) is abs(dFdx(value)) + abs(dFdy(value))
  return smoothstep(threshold - w, threshold + w, value);
}

void main() {
  vec2 coord = gl_PointCoord - vec2(0.5);
  float r = 2.0 * length(coord);

//  color_out = vec4(1.0, 0.0f + 0.001 * color_ex.r, r, 1.0f);

  if(r > 1.1){
    discard;
  }

  color_out_.rgb = color_ex.rgb;
  color_out_.a = 1.0f - aastep(1.0, r);

}
