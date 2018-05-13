#version 330

uniform sampler2D color_sampler_;

/**
 * Inputs from vertex shader
 */
in vec2 coord_;
in vec4 color_ex_;
in vec3 light_;

// This is the up/down projection on the map?
in vec2 eye_relative_;

out vec4 color_out_;


const float M_PI = 3.14159265358979323846;
const float HALF_PI = 0.5f * M_PI;
const float OVER_PI = 1.0f / M_PI;
const float OVER_2PI = 0.5f * OVER_PI;

float scale = 1. / 1536;

float aastep (float threshold , float value) {
  float w =  length ( vec2(dFdx(value), dFdy(value)));
  // GLSL 's fwidth(value) is abs(dFdx(value)) + abs(dFdy(value))
  return smoothstep(threshold - w, threshold + w, value);
}

void main(){

  vec3 light_dir;
  light_dir = normalize(vec3(1.0, 1.0, 0.0));
//  light_dir = normalize(sun_);

  color_out_ = vec4(0.0f);

  vec3 r = vec3(0.0);

  r.xy = 2.0 * (coord_.xy);

  float xy_sq = dot(r.xy, r.xy);

  r.z = sqrt(1 - xy_sq);

  float r_proj_length = sqrt(xy_sq);
  float r_length = sqrt(xy_sq + r.z * r.z);

  if(r_proj_length > 1.1){
    discard;
  }

  color_out_.a = 1.0f - aastep(1.0, r_proj_length);

  vec3 normal = r / r_length;

  vec2 tex_coords;

  tex_coords.x = 0.5 + OVER_2PI * atan(r.z, r.x);
  tex_coords.y = 0.5 - OVER_PI * asin(r.y);

  // Compensate for viewer
  //tex_coords.xy = tex_coords.xy + eye_relative_.xy;

  vec2 dTdx = vec2(-OVER_2PI/r.z, 0.0) * scale;
  vec2 dTdy = vec2(OVER_2PI * r.x * r.y / ((r.y * r.y - 1) * r.z), OVER_PI / sqrt(1-r.y*r.y)) * scale;

//  vec3 moon_color = texture(color_, tex_coords).rgb;
  color_out_.rgb = texture(color_sampler_, tex_coords).rgb;

//  color_out_ = color_out_ * max(0.5f, pow(dot(light_, r), 3));

//  color_out_ = texture(color_sampler_, coord_);
//  color_out_ = vec4(1.0, 0.0, 0.0, 1.0f);
}
