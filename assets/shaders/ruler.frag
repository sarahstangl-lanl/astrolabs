#version 330
// http://www.yaldex.com/open-gl/ch17lev1sec4.html

in vec4 color_ex;

in vec2 tex_;
in vec2 tex_normalized_;

out vec4 color_out;

// Number of labels in the texture
int n_labels = 7;
float ds_label = 1.0 / float(n_labels);

uniform sampler2D labels_;

float ds_stripe = 0.010;
float ds_third_tile = 0.05;
float ds_minor_tile = 0.25;
float ds_major_tile = 1.0;

float tick_length_tiny = 0.2;
float tick_length_small = 0.3;
float tick_length_big = 0.4;

#if 0
float pulse(float edge0, float edge1, float x){
  return step(edge0, x) - step(edge1, x);
}
#else

// Some rudimentary anti-aliasing but still has an issue
float aastep (float threshold , float value) {
  float w =  length ( vec2(dFdx(value), dFdy(value)));
  // GLSL 's fwidth(value) is abs(dFdx(value)) + abs(dFdy(value))
  return smoothstep(threshold - w, threshold + w, value);
}

float pulse(float edge0, float edge1, float x){
  return aastep(edge0, x) - aastep(edge1, x);
}
#endif

float pulsetrain(float edge, float period, float x){
  return pulse(edge, period, mod(x, period));
}

void main() {
  color_out = color_ex;
  float weight = 1.0;

  /**
   *  Draw the tick-marks
   */

  // Small (0.25)
  if(tex_normalized_.y > (1.0 - tick_length_small)){
    weight = min(weight, pulsetrain(ds_stripe, ds_minor_tile, tex_.x));
  }

  // Big (1.0)
  if(tex_normalized_.y > (1.0 - tick_length_big)){
    weight = min(weight, pulsetrain(ds_stripe, ds_major_tile, tex_.x));
  }

  // Tiny (0.05)
  if(tex_normalized_.y > (1.0 - tick_length_tiny)){
    weight = min(weight, pulsetrain(ds_stripe, ds_third_tile, tex_.x));
  }

//  // Rotation handles
//
//  float dx = tex_normalized_.x - 3.0 * ds_label;
//  float dy = tex_normalized_.y - 0.5f;
//
//  float dr = dx * dx + dy * dy;
//
//  if(dr < 0.01){
//    weight = 0.0f;
//  }
//


  // Offset of the big label from the bottom
  vec2 label_offset_big = vec2(0.0, 0.0);
  vec2 label_offset_small = vec2(0.0, 0.0);

//  color_out.rgb = vec3(weight);
// color_out.r = texture(labels_, tick_cs_).r;
  color_out = vec4(0.0, 0.0, 0.0, 1.0);

  // How many big tick marks
  float ds_big = 1.0 / 6.0;
  vec2 local_cs_AU = vec2(0.0, 0.0);

  // This is a normalized coordinate for each 1 AU sized segment
  local_cs_AU.x = min(mod(tex_normalized_.x, ds_big) / ds_big, 1.0);
  local_cs_AU.y = min(1.0, tex_normalized_.y);

  int tick_num_ = int(tex_normalized_.x / ds_big);

  /**
   *
   * Draw the Large Labels
   *
   */

  // Coordinates for the tile
  vec2 label_tex_cs = vec2(0);

  // We grab the tile and then read off ds_label width from it.
  // (NOTE: this 0.75 is sort of a fudge factor to unsquish the large labels)
  label_tex_cs.x = tick_num_ * ds_label + min(0.75 * local_cs_AU.x, ds_label);
  //
  label_tex_cs.y = min(0.55 - local_cs_AU.y, 0.48);

  vec4 color_large = texture(labels_, label_tex_cs);


  /**
   * Draw the Small Labels
   */
  int n_labels_small = 3;

  float local_cs_AU_x = max(0.0, local_cs_AU.x - 0.15); // min(0.5 + local_cs_AU.x, 1.0);
//  float local_cs_AU_y = max(0.0, local_cs_AU.y - 0.15); // min(0.5 + local_cs_AU.x, 1.0);

  float ds_small = 1.0 / float(n_labels_small+1);
  float x_local = min(mod(local_cs_AU_x, ds_small) / ds_small, 1.0);

  int small_tick_num_ = int(local_cs_AU_x/ ds_small);

  float x_small_tile = small_tick_num_ * ds_label + min(x_local* ds_big, ds_label);
  float y_small_tile =  max(1.35 - local_cs_AU.y, 0.49);

  // We only have 3 tiles
  x_small_tile = min(x_small_tile, 2.9 * ds_label);

  vec4 color_small = texture(labels_, vec2(x_small_tile, y_small_tile));

//  color_out.rgb = min(color_small, vec3(weight));
//  color_out.a = 0.5;

  /**
   *   Draw the AU symbol:
   *
   *    Bottom half
   *
   */
  vec4 color_AU = vec4(0.0f);

  if((tick_num_ == 3) && (tex_normalized_.y < 0.5)){

    float x_AU_tile = 6 * ds_label + min(0.75 * max(0, local_cs_AU.x - 0.25), ds_label);
    float y_AU_tile = min(0.40 - local_cs_AU.y, 0.48);

    color_AU.a = texture(labels_, vec2(x_AU_tile, y_AU_tile)).a;

//    color_AU.a = 1.0;
  }


//  if((tex_normalized_.y < 0.5) && (tex_.x > 2.75) && (tex_.x < 3.25)){
//   // color_AU.a = 1.0f;
//
//
////  float x_au_tile = 1 * ds_label + min(x_local* ds_big, ds_label);
////  float y_au_tile =  max(1.35 - local_cs_AU.y, 0.49);
////
////  // We only have 3 tiles
////  x_au_tile = min(x_au_tile, 2.9 * ds_label);
//
//  color_AU.a = texture(labels_, vec2(tex_normalized_.x, tex_normalized_.y)).a;
//
//
////
////    float tex_cs = (tex_.x - 2.75) / 0.5;
////
////
////
////    float x_au_tile = 4 * ds_label + tex_cs * ds_label;// + min(x_local * ds_big, ds_label);
////    // We only have 3 tiles
////    x_au_tile = min(x_au_tile, 2.9 * ds_label);
////
//////    color_AU.a = texture(labels_, vec2(x_au_tile, y_small_tile)).a;
//    color_AU.a = y_small_tile;
//  }

  /**
   *  Composite Text
   */
  float text_alpha = max(color_large.a, color_small.a);
  text_alpha = max(text_alpha, color_AU.a);
  text_alpha = max(1.0 - weight, text_alpha);

  vec4 color_background = vec4(1.0f, 1.0f, 1.0f, 0.5f);
  vec4 color_text = vec4(1.0f, 0.0f, 0.0f, 0.5f);

  color_out = mix(color_background, color_text, text_alpha);


//  color_out = mix(color_out, color_large, color_large.a);
//  color_out = mix(color_out, color_small, color_small.a);

//  color_out.rgb = min(color_large, vec3(weight));
//  color_out.rgb = min(color_out.rgb, color_small);

}
