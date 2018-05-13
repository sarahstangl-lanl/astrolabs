/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: This is the 'meat' of the program
 *
 *
 * TODO: Orbits are not interpolated between elements which limits animation speed.
 * TODO: Antialiasing for Ruler, and arrow
 *
 *
 */


#ifndef KEPLER_SCENE_GRAPH_H
#define KEPLER_SCENE_GRAPH_H

#include <iostream>
#include <math.h>
#include <memory>
#include <vector>

#include "scene_graph.h"

#include <QImage>
#include <QPainter>
#include <QPushButton>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>

#define QT_NO_OPENGL_ES_2
#include <QtWidgets/QOpenGLWidget>

/**
 * Computes an orbital path around a central mass...
 */
template <class T>
struct Orbit{

  // Distance before terminating
  float dr_terminate_ = 0.0001f;

  // Size of the sun
  float R_sun_sq_ = 0.01f;

  /*
   * Orbit States
   */
  bool closed_ = false;
  bool collision_ = false;

  // Figuring out when the orbit actually closes is annoying because
  // both CW and CCW should work and a few corner cases 90 degree.
  // The method here checks distance to start to determine closure but
  // only after particle has passed the Y=0 line. Before it used to
  // compute the angle but that sort of fails for 90 degree orbits.
  bool passed_zero_ = false;


  /**
   *  We are using units such that:
   *    [L] = 1 AU
   *    [M] = Solar Mass
   *    [t] = 1 year
   *
   * then
   *
   *     G = 4 * pi * pi AU^3 yr^{-2} M^-1
   *
   * A perfectly circular orbit with R = 1 AU, centered on the Sun requires
   *
   *  v = sqrt(G)
   *    = 2 * PI AU/yr
   *    = 6.283185 AU/yr
   *    = 29.80565 km/s
   *
   * but we want the circle velocity to be 30km/s so our kps_in_AU_year
   * is slightly faked from
   *
   *  1 km/s = 0.210805 AU/year
   *
   */

  // Gravitational Constant
  T G_ = T(4 * M_PI * M_PI);

  // fudged velocity conversion from km/s to AU/year ( / 30
  T velocity_scale_ = T(2.0 * M_PI / 30.0);

  // Simulation step size is 1 day
  T dt_ = T(1.0 / 365.25);

  struct OrbitPiece {

    T t;
    T x;
    T y;

    T theta;
    T r;
    T area;

    T v_x;
    T v_y;

    void set(float t_in, float x_in, float y_in, float v_x_in, float v_y_in, float theta_in, float area_in){
      t = t_in;
      x = x_in;
      y = y_in;
      theta = theta_in;
      r = sqrt(x * x + y * y);

      v_x = v_x_in;
      v_y = v_y_in;
    }

    std::ostream& operator<<(std::ostream& os){
      os<<t<<" "<<x<<" "<<y<<" "<<theta<<" "<<v_x<<" "<<v_y<<" "<<" "<<r<<std::endl;
      return os;
    }
  };

  OrbitPiece *elements_;

  int element_count_;

  // These are set in the constructor!
  int steps_max_ = 0;
  int sub_steps_ = 0;

  T t_ = T(0);
  T t_max_ = T(0);

  Orbit(int steps_max = 100000, int sub_steps = 10)
      : steps_max_(steps_max), sub_steps_(sub_steps){

    elements_ = new OrbitPiece[steps_max];

  }

  void calculate_force(const T r[], T a[]) {

    T r_ab_dist = 0.0;
    for (int i = 0; i < 2; ++i) {
      r_ab_dist += pow(r[i], 2);
    }

    r_ab_dist = sqrt(r_ab_dist);

    T scale = G_ / (r_ab_dist * r_ab_dist * r_ab_dist);

    for (int i = 0; i < 2; ++i) {
      a[i] = -r[i] * scale;
    }
  }

  /**
   *
   * Orbit::calculate_orbit
   *
   *
   * @param theta_launch_0 initial angle in radians
   * @param v_0 [km/s]
   *
   * @return true if the orbit closes
   */

  void calculate_orbit(float theta_launch_0, float v_0) {

    using namespace std;

    const int nd = 2;

    // Clear state
    closed_ = false;
    collision_ = false;
    passed_zero_ = false;

    T a[nd] = {0.0, 0.0};
    T a_prev[nd] = {0.0, 0.0};
    T v[nd] = {v_0 * velocity_scale_ * cos(theta_launch_0),
               v_0 * velocity_scale_ * sin(theta_launch_0)};
    T r[nd] = {0.0, 1.0};

    element_count_ = 0;
    t_ = T(0);

    T dt_substep = dt_ / float(sub_steps_);

#if 0
    bool cw_direction = (theta_launch_0 <= 90) || (theta_launch_0 >= 270);

    std::cout<<"Orbit::calculate_orbit: Launching with V_0 = "<<v_0
             <<" and theta = "<<theta_launch_0
             <<" clockwise "<<cw_direction
             <<" dt_ "<<dt_
             <<" dt_substep "<<dt_substep
             << " sub_steps_ "<<sub_steps_
             << " steps_max_ "<<steps_max_
             <<std::endl;
#endif

    // The prev is used later...
    calculate_force(r, a);

    for (int step = 0; element_count_ < steps_max_; ++step) {

      for(int sub_step = 0; sub_step < sub_steps_; ++sub_step){

        T theta = atan2(r[1], r[0]);

        elements_[element_count_].set(t_, r[0], r[1], theta, v[0], v[1], 0.0f);

        // Integrate to new position
        for (int i = 0; i < nd; ++i) {
          a_prev[i] = a[i];
          r[i] += v[i] * dt_substep + 0.5 * a[i] * dt_substep * dt_substep;
        }

        calculate_force(r, a);

        for (int i = 0; i < nd; ++i) {
          v[i] += 0.5f * (a[i] + a_prev[i]) * dt_substep;
        }

        t_ += dt_substep;
      }

      passed_zero_ = passed_zero_ || r[1] < 0.0f;

      if(test_terminate(element_count_)){
        // If the distance from the start is less than one step size then the orbit closes
        break;
      }
      element_count_++;
    }

    t_max_ = elements_[element_count_ - 1].t;
  }


  /**
   *
   * Orbit::test_terminate
   *
   * Distance from start
   * @param r
   * @return true if the simulaton
   */
  bool test_terminate(const int element_ix){

    float x = elements_[element_ix].x;
    float y = elements_[element_ix].y;

    // Check if we hit the earth
    float dr_sq_center =  x * x + y * y;

    // If planet impacted
    collision_ = dr_sq_center < R_sun_sq_;

    if(passed_zero_){
      float dx = elements_[0].x - x;
      float dy = elements_[0].y - y;
      float dz = 0.0; //elements_[0].z - e.z;
      float dr_sq = dx * dx + dy * dy + dz * dz;

      // Check if we finished the orbit
      closed_ = (dr_sq < dr_terminate_);
    }

    return collision_ || closed_;
  }

  /**
   *
   * @param t_target
   * @param element_ix (output) the nearest element index
   * @param return the element index
   */
  int time_interpolate(const float t_target, float* position){

    int element_ix = 1;

    for(int i = 0; i < element_count_; ++i){
      if(t_target < elements_[i].t){
        element_ix = i + 1;
//        std::cout<<"time_interpolate: "<<i<<" target/element.t " <<t_target<<" : "<<elements_[i].t
//                 <<" element_ix "<<element_ix<<std::endl;
        break;
      }
    }

    position[0] = elements_[element_ix].x;
    position[1] = elements_[element_ix].y;

    return element_ix;
  }

};


/**
 * Arrow shape
 */
struct Arrow : public msg::Node {

  // Dimensions
  float body_height_ = 0.05f;
  float body_width_ = 0.1f;

  // The "length" of the vector [0.0, 1.0]
  float length_ = 0.0f;

  float body_width_min_ = 0.15f;
  float body_width_max_ = 0.45f;

  // Offset of center
  float x_offset = 0.08f;

  // Size of head
  float head_height_ = 0.20f;
  float head_width_ = 0.15f;

  // Position of the center of the head used for hit testing
  float x_tip_;
  float y_tip_;


  float color[4] = {1.0f, 1.0f, 0.5f, 1.0f};

  // Position and Orientation
  float theta_ = 0.0f;

  // The center of the head of the arrow
  float x_head_center_ = 0.0f;

  // Shaders
  const char *vertexShaderSource =
      "#version 330\n"
          "uniform mat4 mvp_;\n"
          "uniform vec4 color_ = vec4(1.0, 1.0, 0.0, 0.5);\n"
          "uniform float theta_ = 0.0;\n"
          "uniform vec3 offset_ = vec3(0.0f, 1.0f, 0);\n"
          "in vec3 position_;\n"
          "out vec4 color_ex;\n"
          "void main() {\n"
          "   color_ex = color_;\n"
          "   vec3 position = vec3(0.0);\n"
          "   float ct = cos(theta_);\n"
          "   float st = sin(theta_);\n"
          "   position.x = ct * position_.x - st * position_.y;\n"
          "   position.y = st * position_.x + ct * position_.y;\n"
          "   gl_Position = mvp_ * vec4(position + offset_, 1.0);\n"
          "}\n";

  const char *fragmentShaderSource =
      "#version 330\n"
          "\n"
          "in vec4 color_ex;\n"
          "out vec4 color_out;\n"
          "\n"
          "void main() {\n"
          "  color_out = color_ex;\n"
          "}\n";

  // Drawing indices
  const static int count_ = 9;
  const unsigned int const_ix_[count_] = {0, 1, 2, 2, 3, 1, 4, 5, 6};

  float vertices_[7][3];


  // Program handle
  GLuint program_;

  // Position Attribute
  GLuint positionHandle_;

  // Uniform handles
  GLint mvpHandle_;

  GLint colorHandle_;
  GLint offsetHandle_;
  GLint thetaHandle_;

  GLuint vao = 0;
  GLuint vbo = 0;

  Arrow(){

    for(int i = 0; i < 3; ++i){
      for(int j = 0; j < 7; ++j){
        vertices_[j][i] = 0.0f;
      }
    }

    setPosition(0.0f, 1.0f, 0.0f);
  }

  /**
   * Arrow::init_resources
   *
   */
  bool init_resources(){

    program_ = glCreateProgram();

    // Build programs
    bool status = build_program_from_source(program_, "arrow", vertexShaderSource, fragmentShaderSource);

    if(!status){
      return false;
    }

    positionHandle_ = glGetAttribLocation(program_, "position_");
    mvpHandle_ = glGetUniformLocation(program_, "mvp_");
    colorHandle_ = glGetUniformLocation(program_, "color_");
    offsetHandle_ = glGetUniformLocation(program_, "offset_");
    thetaHandle_ = glGetUniformLocation(program_, "theta_");

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // Send the zeroed-out buffer
    glBufferData(GL_ARRAY_BUFFER, 21 * sizeof(float), vertices_, GL_STATIC_DRAW);

    glVertexAttribPointer(positionHandle_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionHandle_);

    buildArrow();

    return check_GL_error("Arrow::init_resources() exit");
  }

  void cleanup(){
    glDeleteProgram(program_);

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

  }

  /**
   *
   * Arrow::set_angle
   *
   * @param theta Rotation angle in radians
   */
  void set_angle(float theta){
    theta_ = theta;
    update_position();
  }


  /**
   * Arrow::set_length
   *
   * @param length
   */
  void set_length(float length){
    length_ = length;
    body_width_ = body_width_min_ + length_ * body_width_max_;
    buildArrow();
  }

  /**
   * Arrow::update_position
   *
   * @param length
   */
  void update_position(){
    x_tip_ = position[0] +  x_head_center_ * cos(theta_);
    y_tip_ = position[1] +  x_head_center_ * sin(theta_);
  }

  /**
   * Arrow::build_arrow
   *
   */
  void buildArrow(){

    using namespace std;

    // Left edge of body
    float x_left = -x_offset;

    // Right edge of body
    float x_right = body_width_ - x_offset;

    // TODO: Need to go some proportion of this because it's a triangle
    x_head_center_ = x_right + 0.25 * head_width_;

    float z_offset = 0.0f;

    float half_height = 0.5f * body_height_;
    float half_head_height = 0.5f * head_height_;

    vertices_[0][0] = x_left;
    vertices_[0][1] = half_height;
    vertices_[0][2] = z_offset;

    vertices_[1][0] = x_right;
    vertices_[1][1] = half_height;
    vertices_[1][2] = z_offset;

    vertices_[2][0] = x_left;
    vertices_[2][1] = -half_height;
    vertices_[2][2] = z_offset;

    vertices_[3][0] = x_right;
    vertices_[3][1] = -half_height;
    vertices_[3][2] = z_offset;

    vertices_[4][0] = x_right;
    vertices_[4][1] = half_head_height;
    vertices_[4][2] = z_offset;

    // Tip of the head
    vertices_[5][0] = x_right + head_width_;
    vertices_[5][1] = 0.0f;
    vertices_[5][2] = z_offset;

    vertices_[6][0] = x_right;
    vertices_[6][1] = -half_head_height;
    vertices_[6][2] = z_offset;
//
//    const float vertices[7][3]{{x_left,  half_height, 0.0f},
//                               {x_right,  half_height, 0.0f},
//                               {x_left, -half_height, 0.0f},
//                               {x_right, -half_height, 0.0f},
//                               {x_right, half_head_height, 0.0f},
//                               {x_right + head_width_, 0.0f, 0.0f},
//                               {x_right, -half_head_height, 0.0f}};

#if 0
    std::cout<<"Arrow::build_arrow: Arrow vertices"<<std::endl;
    for(int i = 0; i < 7; ++i){
      cout<<i<<" v "<<vertices_[i][0]<<", "<<vertices_[i][1]<<", "<<vertices_[i][2]<<endl;
    }
#endif

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices_), vertices_);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    update_position();

  }

  /**
   *
   * Arrow::hitTest  - Test for intersection
   *
   *
   * @param x
   * @param y
   * @return
   */
  virtual bool hitTest(float x, float y){

    using namespace std;

    float r_head_sq = 0.25 * head_width_ * head_width_;

    // Find the location of the arrow tip

    float dx = x_tip_ - x;
    float dy = y_tip_ - y;

    float r_sq = dx * dx + dy * dy;

    return r_sq < r_head_sq;
  }

  /**
   *
   * Arrow::handleDrag
   *
   * @param x_i
   * @param x_f
   * @param y_i
   * @param y_f
   *
   */
  virtual bool handleDrag(float x_i, float y_i, float x_f, float y_f){
    using namespace std;

//    std::cout<<"Arrow::handleDrag x="<<x_i<<" ... "<<x_f<<"y="<<y_i<<" ... "<<y_f<<std::endl;

    float cos_t = cos(-theta_);
    float sin_t = sin(-theta_);

    // Offset
    float dx_center_i = position[0] - x_i;
    float dy_center_i = position[1] - y_i;
    // Rotation
    float dx_i = dx_center_i * cos_t - dy_center_i * sin_t;
    float dy_i = dx_center_i * sin_t + dy_center_i * cos_t;

    // Offset
    float dx_center_f = position[0] - x_f;
    float dy_center_f = position[1] - y_f;
    // Rotation
    float dx_f = dx_center_f * cos_t - dy_center_f * sin_t;
    float dy_f = dx_center_f * sin_t + dy_center_f * cos_t;


    // This should be small enough to where we can just take it as is, maybe.
    float dx = dx_f - dx_i;
    float dy = dy_f - dy_i;

    // Set the length
    length_ -= dx;

    // Clamp it
    if(length_ < 0.0f) length_ = 0.0f;
    if(length_ > 1.0f) length_ = 1.0f;

    body_width_ = body_width_min_ + length_ * body_width_max_;

    /**
     * Compute the angle
     */

    // Ideally we would use the distance between the click point and the
    // base of the arrow as the distance R.
    float R = body_width_;
//    float R  = x_head_center_;

    // Set the angle
    float dtheta = atan(dy / R);

    theta_ -= dtheta;

    if(theta_ > 2.0f * M_PI){
      theta_ -= float(2.0f * M_PI);
    }else if(theta_ < -2.0f * M_PI){
      theta_ += float(2.0f * M_PI);
    }

    buildArrow();

    return true;
  }


  /**
   *
   * Arrow::render
   *
   * @param camera_
   */
  void render(Camera<float>& camera_){

    // Draw
    glUseProgram(program_);
    glBindVertexArray(vao);
    glUniformMatrix4fv(mvpHandle_, 1, GL_FALSE, camera_.mvp);
    glUniform1f(thetaHandle_, theta_);
    glUniform3fv(offsetHandle_, 1, position);

    glDrawElements(GL_TRIANGLES, count_, GL_UNSIGNED_INT, const_ix_);
    glBindVertexArray(0);
  }

};

/**
 * The on-screen ruler
 */
struct Ruler : public msg::FlatShape {

  /**
   * Settings
   */

  // For drawing the main stuff
  const int default_font_size_ = 12;
  // For drawing the superscript
  const int default_font_sm_size_ = 8;

  /**
   * The actual font sizes which will be scaled
   * up or down.
   */
  int font_size_large = default_font_size_;
  int font_size_small = default_font_sm_size_;

  // Dimensions
  float size_[2] = {6.03f, 0.5f};
//  float color[4] = {1.0f, 1.0f, 0.5f, 0.2f};

  // Position and Orientation
  float position_[3] = {0, 0, 0};
  float angle_ = 0;
  float r_mat_[16];
  float inv_r_mat_[16];

  // The handles for spinning the ruler
  float handle_radius = 0.1f;
  float handle_offset_x = 1.5f;
  float handle_offset_y = 0.0f;

  float handle_position[2][3];

  // Shaders
  const char *vertexShaderFilename = "./assets/shaders/ruler.vert";
  const char *fragmentShaderFilename = "./assets/shaders/ruler.frag";

  // Program handle
  GLuint program_;

  // Position Attribute
  GLuint positionHandle_;

  // Uniform handles
  GLint mvpHandle_;
  GLint rHandle_;

  GLint colorHandle_;
  GLint offsetHandle_;
  GLint sizeHandle_;
  GLint labelsHandle_;

  // Size of the label texture
  const static int max_label_w_ = 64;
  const static int max_label_h_ = 64;

  const static int max_labels_x_ = 7;
  const static int max_labels_y_ = 2;

  // If this is 1.0 then front handle was hit if this is < -1.0 then rear handle was hit,
  // in the future this can be used to do some gradual rotation not on handles
  float handle_hit_ = 0.0f;

  /**
   * Label Generator for the ruler
   */
  msg::ImageAtlas atlas_;


  /**
   * Default constructor doesn't do much
   */
  Ruler(int tex_unit)
      :  atlas_(tex_unit, max_label_w_, max_label_h_, max_labels_x_, max_labels_y_){

    reset();
  }

  /**
   * Ruler::init_resources
   *
   * @return
   */
  bool init_resources(){

    // Initialize the geometry
    msg::FlatShape::init_resources();
    build_quad(size_[0], size_[1]);

    program_ = glCreateProgram();

    // Build programs
    if(!build_program(program_, "Ruler", vertexShaderFilename, fragmentShaderFilename)){
      return false;
    }

    positionHandle_ = glGetAttribLocation(program_, "position");
    mvpHandle_ = glGetUniformLocation(program_, "mvp_");
    rHandle_ = glGetUniformLocation(program_, "r_");
    colorHandle_ = glGetUniformLocation(program_, "color_");
    offsetHandle_ = glGetUniformLocation(program_, "offset_");
    sizeHandle_ = glGetUniformLocation(program_, "size_");
    labelsHandle_ = glGetUniformLocation(program_, "labels_");

    setup_array(positionHandle_);

    atlas_.init_resources();
    generateLabels();

    return check_GL_error("Ruler::build() exit");
  }

  /**
   * Ruler::reset()
   *
   *   Set defaults
   *
   */
  void reset(){
    set_transform(0.025f, 0.0f, 0.0f * M_PI);
  }

  /**
   * Ruler::cleanup
   */
  void cleanup(){
    glDeleteProgram(program_);
  }

  /**
   *  Ruler::generate_labels
   *
   */
  void generateLabels(){

    using namespace std;
    check_GL_error("RulerLabelAtlas::generate_labels() entry");

    // Two sizes of labels
    const static int label_count = max_labels_x_;
    const static int small_label_count = 3;

    const char *labels[] = {"0", "1", "2", "3", "4", "5", "AU",
                            "0.25", "0.50", "0.75"};

//    // Background color
//    QColor C_background = Qt::yellow;
//
//    // The color of the border
//    QColor C_padding = Qt::red;
//
    // Background color
    QColor C_background = Qt::transparent;

    // The color of the border
    QColor C_padding = Qt::transparent;

//    // The color of the
//    QColor C_padding = Qt::transparent;

    // Bind the texture
    atlas_.bind();

    QImage image(max_label_w_, max_label_h_, QImage::Format_RGBA8888);
    image.fill(0);
    QPainter painter(&image);

    painter.fillRect(image.rect(), C_padding);
    QFont font_large = painter.font();
    font_large.setPointSize(font_size_large);
    font_large.setWeight(QFont::DemiBold);

    QFont font_small = painter.font();
    font_small.setPointSize(font_size_small);
    font_small.setWeight(QFont::DemiBold);

    // Draw large labels
    for(int i = 0; i < label_count + small_label_count; ++i){

      for(int x = 0; x < image.width(); ++x){
        for(int y = 0; y < image.height(); ++y){
          image.setPixel(x, y, qRgba(0, 0, 0, 0));
        }
      }

      painter.fillRect(QRect(1, 1, max_label_w_-1, max_label_h_-1), C_background);

      // We use 2 different font sizes
      painter.setFont( i < label_count ? font_large : font_small);

      int align_flag = i < label_count ? Qt::AlignLeft : Qt::AlignCenter;

      painter.drawText(QRect(1, 1, max_label_w_-1, max_label_h_-1),
                       align_flag | Qt::AlignBaseline,
                       labels[i]);

      // Upload the tile to the texture unit
      atlas_.add_tile(image.bits());
    }

#if 0
    atlas_.save_atlas("ruler_label_atlas.png");
#endif
    check_GL_error("RulerLabelAtlas::generate_labels() exit");
  }


  /**
   *
   * Ruler::set_transform
   *
   * Position of center and angle to horizon
   *
   * @param x
   * @param y
   * @param angle
   */
  void set_transform(float x, float y, float angle){

 //   std::cout<<"Ruler::set_transform "<<std::endl;

    // The center of the ruler in global CS
    position_[0] = x;
    position_[1] = y;

    // Angle of the ruler from x-axis
    angle_ = angle;

    // Build a rotation matrix

    // Initialize an identity matrix
    for(int i = 0; i < 16; ++i){
      r_mat_[i] = i % 5 == 0 ? 1.0f : 0.0f;
      inv_r_mat_[i] = r_mat_[i];
    }

    r_mat_[0] = cos(angle_);
    r_mat_[1] = sin(angle_);
    r_mat_[4] = -r_mat_[1];
    r_mat_[5] = r_mat_[0];

    // The inverse matrix
    inv_r_mat_[0] = cos(angle_);
    inv_r_mat_[1] = -sin(angle_);

    inv_r_mat_[4] = -inv_r_mat_[1];
    inv_r_mat_[5] = inv_r_mat_[0];

    // Update the handles
    handle_position[0][0] = position_[0] + handle_offset_x * r_mat_[0];
    handle_position[0][1] = position_[1] + handle_offset_x * r_mat_[1];
    handle_position[0][2] = 0.0f;

    handle_position[1][0] = position_[0] - handle_offset_x * r_mat_[0];
    handle_position[1][1] = position_[1] - handle_offset_x * r_mat_[1];
    handle_position[1][2] = 0.0f;
  }

  /**
   *
   * Ruler::hitTest
   *
   *
   * @param x Position of click in world space.
   * @param y Position of click in world space.
   * @return
   */
  virtual bool hitTest(float x, float y){

    using namespace std;

    // Offset
    float dx_center = x - position_[0];
    float dy_center = y - position_[1];

    // Rotation
    float dx_rotated = dx_center * r_mat_[0] + dy_center * r_mat_[1];
    float dy_rotated = dx_center * r_mat_[4] + dy_center * r_mat_[5];

    bool hit = (abs(dx_rotated) < 0.5 * size_[0]) && (abs(dy_rotated) < 0.5 * size_[1]);

//    std::cout<<"Ruler::hitTest  "<<x<<", "<<y<<std::endl;
//    std::cout<<" position_[0] "<<position_[0]<<endl;
//    std::cout<<" position_[1] "<<position_[1]<<endl;
//    std::cout<<" dx_center "<<dx_center<<endl;
//    std::cout<<" dy_center "<<dy_center<<endl;
//    std::cout<<" dx_rotated "<<dx_rotated<<endl;
//    std::cout<<" dy_rotated "<<dy_rotated<<endl;
//    std::cout<<" 0.5 * size_[0] "<<0.5 * size_[0]<<endl;
//    std::cout<<" 0.5 * size_[1] "<<0.5 * size_[1]<<endl;
//
//    if(hit){
//      std::cout<<" hit: "<<hit<<endl;
//    }else{
//      std::cout<<" miss: "<<hit<<endl;
//    }

    /**
     * Check if a handle has been hit
     */

    handle_hit_ = 0.0f;
    float r_sq_handle = handle_radius * handle_radius;

    // Distance from handle 1
    float dx_from_handle = handle_position[0][0] - x;
    float dy_from_handle = handle_position[0][1] - y;
    float dr_sq = dx_from_handle * dx_from_handle + dy_from_handle * dy_from_handle;

    if(dr_sq < r_sq_handle){
      handle_hit_ = 1.0;
    }

    // Distance from handle 2
    dx_from_handle = handle_position[1][0] - x;
    dy_from_handle = handle_position[1][1] - y;

    dr_sq = dx_from_handle * dx_from_handle + dy_from_handle * dy_from_handle;

    if(dr_sq < r_sq_handle){
      handle_hit_ = -1.0;
    }

    return hit;
  }

  /**
   * Ruler::handleDrag
   *
   * @param x_i
   * @param x_f
   * @param y_i
   * @param y_f
   *
   */
  virtual bool handleDrag(float x_i, float y_i, float x_f, float y_f){
    using namespace std;

    // Offset
    float dx_center = x_i - position_[0];
    float dy_center = y_i - position_[1];

    // How much are we moving
    float dx_screen = x_f - x_i;
    float dy_screen = y_f - y_i;

    // Rotation
    float dx_rotated = dx_center * r_mat_[0] + dy_center * r_mat_[1];
    float dy_rotated = dx_center * r_mat_[4] + dy_center * r_mat_[5];

    bool hit = (abs(dx_rotated) < 0.5 * size_[0]) && (abs(dy_rotated) < 0.5 * size_[1]);

    float dx_move = dx_screen;
    float dy_move = dy_screen;

    float dt = 0.0f;

//    // Distance from handle 1
//    float dx_from_handle = handle_position[0][0] - x_i;
//    float dy_from_handle = handle_position[0][1] - y_i;
//    float r_sq_handle = handle_radius * handle_radius;
//    float dr_sq = dx_from_handle * dx_from_handle + dy_from_handle * dy_from_handle;
//
//
//    // If we are dragging a handle
//    if(dr_sq < r_sq_handle){
//      dt = atan2(dy_rotated, handle_offset_x);
//      dx_move = 0.0f;
//      dy_move = 0.0f;
//    }
//
//    // Distance from handle 2
//    dx_from_handle = handle_position[1][0] - x_i;
//    dy_from_handle = handle_position[1][1] - y_i;
//    r_sq_handle = handle_radius * handle_radius;
//
//    dr_sq = dx_from_handle * dx_from_handle + dy_from_handle * dy_from_handle;
//
//    // If we are dragging a handle
//    if(dr_sq < r_sq_handle){
//      dt = -atan2(dy_rotated, handle_offset_x);
//      dx_move = 0.0f;
//      dy_move = 0.0f;
//    }

    // If we are dragging a handle ( TODO: Cleanup )
    if((handle_hit_ > 0.0f) || (handle_hit_ < 0.0f)){
      dt = handle_hit_ * atan2(dy_rotated, handle_offset_x);
      dx_move = 0.0f;
      dy_move = 0.0f;
    }

    float x_new = position_[0] + dx_move;
    float y_new = position_[1] + dy_move;

    set_transform(x_new, y_new, angle_ + dt);

//    std::cout<<"Ruler::handleDrag  "<<x_i<<", "<<y_i<<std::endl;
//    std::cout<<" angle_ + dtheta "<<angle_ + dt<<endl;
//    std::cout<<" position_[0] "<<position_[0]<<endl;
//    std::cout<<" position_[1] "<<position_[1]<<endl;
//    std::cout<<" dx_center "<<dx_center<<endl;
//    std::cout<<" dy_center "<<dy_center<<endl;
//    std::cout<<" dx_rotated "<<dx_rotated<<endl;
//    std::cout<<" dy_rotated "<<dy_rotated<<endl;
//    std::cout<<" dx_from_handle "<<dx_from_handle<<endl;
//    std::cout<<" dy_from_handle "<<dy_from_handle<<endl;
//
//    if(dr_sq < r_sq_handle){
//      std::cout<<"HANDLE!!!"<<endl;
//    }

    return hit;
  }


  /**
   *
   *  Ruler::render
   *
   * @param camera_
   */
  virtual void render(Camera<float>& camera){

//    std::cout<<"Ruler::render"<<std::endl;

    // Draw
    glUseProgram(program_);

    // The Model View and Projection matrices. Each `node` has its own MV
    // so we supply the p matrix separately... I guess. Can easily do it offline too.
    glUniformMatrix4fv(mvpHandle_, 1, GL_FALSE, camera.mvp);
    glUniformMatrix4fv(rHandle_, 1, GL_FALSE, r_mat_);

    glUniform3fv(offsetHandle_, 1, position_);
    glUniform2fv(sizeHandle_, 1, size_);
    glUniform1i(labelsHandle_, atlas_.tex_unit_);

//    float size[2] = {x_length_, y_length_};
//    glUniform1f(angleHandle_, angle_);
    msg::FlatShape::bind();
    msg::FlatShape::render(camera);

    check_GL_error("Ruler::render() exit");

  }


};

/**
 * Node for storing and rendering a set of sweeps and their labels. After the orbit is calculated,
 * triangles covering the entire sweep are generated. Indices for covering the entire area twice
 * are also generated so we can render any sweeps through indices rather than uploading geometry.
 *
 */
struct Sweeps : public msg::Node {

  /**
   * Settings
   */
  const float sweep_alpha = 0.75f;

  // Label size in world coordinates
  const float label_scale = 1.5f;
  const float label_height_ = 0.3f * label_scale;
  const float label_width_ = 0.5f * label_scale;

  float R_label_ = 1.50f;

  int max_sweep_count_ = 32;

  // Label size in pixels
  const static int label_image_w_ = 256;
  const static int label_image_h_ = 128;

  // For drawing the main stuff
  const int default_font_size_ = 20;

  // For drawing the superscript
  const int default_font_sm_size_ = 18;

  // For drawing the main stuff
  int font_size_ = default_font_size_;

  // For drawing the superscript
  int font_sm_size_ = default_font_sm_size_;

  // Color catalog for sweeps
  const static int color_count_ = 15;
  const float colors[color_count_][4] = {{1.0f, 0.4f, 0.0f, sweep_alpha},
                                         {1.0f, 0.8f, 0.0f, sweep_alpha},
                                         {0.4f, 1.0f, 0.0f, sweep_alpha},
                                         {0.8f, 0.0f, 1.0f, sweep_alpha},
                                         {1.0f, 0.0f, 0.8f, sweep_alpha},
                                         {0.8f, 1.0f, 0.0f, sweep_alpha},
                                         // Hacked duplicates of above
                                         {1.0f, 0.4f, 0.5f, sweep_alpha},
                                         {1.0f, 0.8f, 0.5f, sweep_alpha},
                                         {0.8f, 1.0f, 0.5f, sweep_alpha},
                                         {1.0f, 0.0f, 0.4f, sweep_alpha},
                                         {0.4f, 1.0f, 0.5f, sweep_alpha},
                                         {0.8f, 0.5f, 1.0f, sweep_alpha},
                                         {0.4f, 0.5f, 1.0f, sweep_alpha},
                                         {1.0f, 0.5f, 0.8f, sweep_alpha},
                                         {1.0f, 0.5f, 0.4f, sweep_alpha}};
  int color_ix_last_ = 0;

  // Shaders
  const char *vert_shader = "./assets/shaders/sweep.vert";
  const char *frag_shader = "./assets/shaders/sweep.frag";

  // Maximum number of elements
  int capacity_ = 0;
  const static int stride_ = 3;

  // Number of used elements
  int size_ = 0;

  // Vertices for the path
  float *data_;

  // Accumulated area swept out by the path area_[0] = area from ix to ix+1;
  float *area_;

  // We want to cover 2 full loops of sweeps so that we can start from anywhere
  unsigned int *index_;

  // Geometry Vertex Array and Vertex Buffer
  GLuint vao = 0;
  GLuint vbo = 0;

  // Program handle
  GLuint program_;

  // Position Attribute
  GLuint positionHandle_;

  // Uniform handles
  GLint mvHandle_;
  GLint pHandle_;

  GLint colorHandle_;
  GLint offsetHandle_;

  struct s_sweep {

    // Index array pointer for start element
    unsigned int *start = nullptr;

    // Start element
    int start_ix;

//    // Start and end angles (For label positioning)
//    float theta_i = 0.0f;
//    float theta_f = 0.0f;

    // Number of elements in s_sweep
    int count = 0;

    float z_offset = 0.0;

    // Total Area
    float total_area = 0.0;

    // Total Area
    float total_time = 0.0;

    // Sweep Color
    float color[4] = {1.0f, 0.0f, 1.0f, 0.5f};

    int label_ix = 0;

//    // Label
//    float label_position[3] = {0.0f, 0.0f, 0.0f};

    s_sweep(int start_ix, unsigned int *start, const float c[], float z_offset = 0.0f)
        : start(start), start_ix(start_ix), z_offset(z_offset){

      for(int i = 0; i < 4; ++i){
        color[i] = c[i];
      }
    }
  };

  // The orbital sweeps information
  std::vector <s_sweep> sweeps_list_;

  // Index of current sweep (last or after roll-over )
  float sweep_ix_ = 0;

  // Number of sweeps sort of, it can be > sweeps_list_.size() since they roll over
  int sweep_count_ = 0;

  // Texture atlas for sweep labels
  msg::ImageAtlas labels_atlas_;

  // Billboard quads for the sweep labels
  msg::BillboardSet labels_billboards_;

  /**
   * Qt Specific stuff
   */
  QColor C_background_ = QColor(80, 80, 80, 180);
  QColor C_padding_ = Qt::transparent;
  QImage label_image_;


  /**
   * Sweeps::Sweeps
   *
   * @param tex_unit
   * @param capacity
   */
  Sweeps(int tex_unit, int capacity)
      : capacity_(capacity),
        labels_atlas_(tex_unit, label_image_w_, label_image_h_, max_sweep_count_),
        labels_billboards_(max_sweep_count_){

    labels_billboards_.quad_width_ = label_width_;
    labels_billboards_.quad_height_ = label_height_;

    data_ = new float[stride_ * capacity_];
    area_ = new float[2 * capacity_];
    index_ = new unsigned int[6 * capacity_];

    clear();
  }

  ~Sweeps(){
    delete[] data_;
    delete[] area_;
    delete[] index_;
  };

  /**
   *
   * Sweeps::init_resources
   *
   * @return
   */
  bool init_resources(){

    program_ = glCreateProgram();

    // Build programs
    if(!build_program(program_, "Sweeps", vert_shader, frag_shader)){
      return false;
    }

    positionHandle_ = glGetAttribLocation(program_, "position");

    mvHandle_ = glGetUniformLocation(program_, "mv_");
    pHandle_ = glGetUniformLocation(program_, "p_");
    colorHandle_ = glGetUniformLocation(program_, "color_");
    offsetHandle_ = glGetUniformLocation(program_, "offset_");

    /**
     * Sweep Geometry
     */
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, stride_ * capacity_ * sizeof(float), nullptr, GL_STATIC_DRAW);

    glVertexAttribPointer(positionHandle_, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(positionHandle_);

    /**
     * Labels
     */

    label_image_ = QImage(label_image_w_, label_image_h_, QImage::Format_RGBA8888);

    labels_atlas_.init_resources();

    labels_billboards_.atlas_tex_unit_ = labels_atlas_.tex_unit_;
    labels_billboards_.init_resources();
    labels_billboards_.upload_billboards();

    return check_GL_error("Sweeps::init_resources() exit");
  }

  /**
   * Sweeps::clear()
   *
   *    Reset the sweep geometry
   */
  void clear(){

    // Add the center point
    data_[0] = 0.0f;
    data_[1] = 0.0f;
    data_[2] = 0.0f;

    size_ = 1;

    sweeps_list_.clear();
    sweep_ix_ = 0;
    sweep_count_ = 0;
    labels_atlas_.count_ = 0;
    labels_billboards_.count_ = 0;
  }

  /**
   * Sweeps::cleanup()
   */
  void cleanup(){
    glDeleteProgram(program_);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  /**
   *
   * Sweeps::addPoint
   *
   *
   *
   * Add an orbit point.
   * @param x
   * @param y
   * @param z
   * @return
   */
  bool addPoint(float x, float y, float z){

    using namespace std;

#if 0
    std::cout << " Sweep Vertices " << size_ << " - " << x << ", " << y << ", " << z << endl;
#endif
    int offset = stride_ * size_;

    if(size_ >= capacity_ - 2){
      cerr << "Sweep Vertices: Not enough space" << endl;
      return false;
    }

    data_[offset] = x;
    data_[offset + 1] = y;
    data_[offset + 2] = z;
    size_ += 1;

    return true;
  }

  /**
   *
   * Sweeps::build_and_upload
   *
   * Call this after adding all the points for the sweeps it will :
   *
   *  - Build index table
   *  - Compute sweep area
   *  - Upload sweep geometry
   *
   */
  void buildUpload(bool closed){

    // First vertex is the center of the sweep.
    int vert_offset = 1;
    int ix = 0;

#if 0
    // Build the index, double the size we actually need
    std::cout << "Sweeps::build_and_upload Building Index " << stride_ * size_ * sizeof(float) << " bytes" << std::endl;
    std::cout << "Sweeps::build_and_upload : build_and_upload: closed "<< closed << std::endl;
    std::cout << "Sweeps::build_and_upload : build_and_upload: size "<< size_ << std::endl;
    float total_area = 0.0f;

#endif

    float area = 0.0f;

    int loop_size = closed ? 2 * size_ : size_ - 3;

    for(int i = 0; i < loop_size; ++i) {

      int a = 0;
      int b = vert_offset;
      vert_offset = std::max(1, (vert_offset + 1) % size_);
      int c = vert_offset;

      index_[ix++] = float(a);
      index_[ix++] = float(b);
      index_[ix++] = float(c);

      if (i > 1) {
        if (i < size_ + 1) {
          // Assume that the center is at the origin, which it is, then A = 0.5 (x_1 y_2 - x_2 y_1)
          // this is the shoelace formula for when one vertex is in the center.
          float dA = 0.5f * (data_[stride_ * b] * data_[stride_ * c + 1]
                             - data_[stride_ * c] * data_[stride_ * b + 1]);
          area += abs(dA);
//          total_area = area;
        } else {
          area = area_[size_ - 1] + area_[i - size_];
        }
      }
      area_[i] = area;
    }

#if 0
    std::cout<<"Sweeps::build_and_upload : Total Area "<<total_area<<std::endl;
    std::cout<<"Sweeps::build_and_upload : Total Size "<<size_<<std::endl;
#endif

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, stride_ * size_ * sizeof(float), data_);
    check_GL_error("s_sweep::upload() - exit");

    // We want the data transfered before we return from this to prevent stutters in the
    // animation later (TODO: Check if this had any effect)
    glFlush();
  }

  /**
   *
   * Sweeps::addSweep
   *
   * Start a sweep at the point given by index ix
   *
   * @param ix
   */
  void addSweep(int orbit_point_ix, float label_x, float label_y){

    // Sweeps are made out of triangles between the center and two points in
    // the orbit ring. To save on memory we store only one copy of all the points
    // including the center point and then construct the triangles by using
    // three indices per triangle. use indexed arrays.
    int start_ix = 3 * orbit_point_ix;

    s_sweep s = s_sweep(start_ix, &index_[start_ix], &colors[color_ix_last_][0]);
    sweep_count_ += 1;

    // If we are going
    if((int)sweeps_list_.size() < max_sweep_count_){
      // Create a new s_sweep
      sweeps_list_.push_back(s);
      sweep_ix_ = int(sweeps_list_.size() - 1);
    }else{
      sweep_ix_ = sweep_count_ % max_sweep_count_;
      sweeps_list_.at(sweep_ix_) = s;
    }

    // Init the texture coordinates
    sweeps_list_[sweep_ix_].label_ix = labels_atlas_.add_tile();
    writeLabel(sweeps_list_[sweep_ix_].label_ix, 0.0f, 0.0f, sweeps_list_[sweep_ix_].color);

    labels_atlas_.get_tile_coordinates(sweep_ix_, labels_billboards_.info_[sweep_ix_].tex);

    color_ix_last_ = (color_ix_last_ + 1) % color_count_;

    if(labels_billboards_.count_ < labels_billboards_.capacity_){
      ++labels_billboards_.count_;
    }

    labels_billboards_.info_[sweep_ix_].position[0] = R_label_ * label_x;
    labels_billboards_.info_[sweep_ix_].position[1] = R_label_ * label_y;

    labels_billboards_.upload_billboards();
    glFlush();
  }

  /**
   *
   * Sweeps::updateSweep
   *
   * During sweeps each animation tick updates the last index for the sweep
   * as well as its label.
   *
   * Several bugs left in this routine:
   *
   *  - Area computation
   *  - Position of label
   *  - Overlap of element
   *
   * These will probably all be resolved if we picked the correct element_ix_f.
   *
   * @param point_ix
   * @return number of orbit points swept this far
   */
  int updateSweep(int point_ix){

    using namespace std;

    int start_ix = sweeps_list_[sweep_ix_].start_ix;

    int element_ix = 1 + 3 * point_ix;

    if(start_ix > element_ix){
      element_ix += 3 * (size_ - 1);
    }

    // TODO: Set the end index for this current s_sweep
    sweeps_list_[sweep_ix_].count = element_ix - start_ix;

    int point_ix_i = start_ix / 3;
    int point_ix_f = element_ix / 3;

    float A_i = area_[point_ix_i];
    float A_f = area_[point_ix_f];

    float area = abs(A_f - A_i);

    float days = sweeps_list_[sweep_ix_].count / 3.0f;

#if 0
    cout<<"Sweeps::updateSweep element_ix_B = "<<element_ix<<endl;
    cout<<"Sweeps::updateSweep sweep.count = "<<sweeps_list_[sweep_ix_].count<<endl;
    cout<<"Sweeps::updateSweep point_ix_i "<<point_ix_i<<", point_ix_f "<<point_ix_f<<endl;
    cout<<"Sweeps::updateSweep A_i "<<A_i<<", A_f "<<A_f<<endl;
    cout<<"Sweeps::updateSweep area = "<<area<<", days = "<<days<<endl;
#endif

    // Regenerate label
    writeLabel(sweep_ix_, area, days, sweeps_list_[sweep_ix_].color);

    return sweeps_list_[sweep_ix_].count / 3;
  }

  /**
   *
   * Sweeps::writeLabel
   *
   * @param label_ix
   * @param area
   * @param days
   * @param color
   */
  void writeLabel(int label_ix, float area, float days, const float color[]){

    using namespace std;

    int x_padding = 10;

    // Bind the texture
    labels_atlas_.bind();
    label_image_.fill(0);

    QPainter painter(&label_image_);

    QPen pen(QColor(255 * color[0], 255 * color[1], 255 * color[2], 255 * color[3]));

    painter.setPen(pen);
    painter.fillRect(label_image_.rect(), C_padding_);

    // Create fonts (TODO: Move out)
    QFont font = painter.font();
    font.setWeight(QFont::Bold);
    font.setPointSize(font_size_);
    QFontMetrics fm(font);

    QFont font_small = painter.font();
    font_small.setPointSize(font_sm_size_);
    font_small.setWeight(QFont::Bold);
    QFontMetrics fm_small(font_small);

    // We have to draw the t =, number and units separately to get the justifying correct.
    const char *label_area = "A =";
    const char *label_time = "t =";

    const char *label_AU = " AU";
    const char *label_days = " days";
    const char *max_value = "9999.00";

    QString area_str = QString::number(area, 'f', 2);
    QString days_str = QString::number(days, 'f', 2);

    // Figure out column widths
    int prefix_width = max(fm.width(label_area), fm.width(label_time));
    int value_width = fm.width(max_value);
    int suffix_width = max(fm.width(label_AU), fm.width(label_days));

    int text_height = fm.height();

    // TODO: Add more "white-space" on the left and right of label

    int y_offset_1 = int(0.5f * fm_small.height() + x_padding);
    int y_offset_2 = y_offset_1 + text_height;


    // Use the big font
    painter.setFont(font);

#if 0
    //
    painter.fillRect(QRect(x_padding, x_padding, prefix_width + value_width + suffix_width,
                           y_offset_2 + text_height), C_background_);
#else
    painter.fillRect(QRect(0, 0, 2 * x_padding + prefix_width + value_width + suffix_width,
                           y_offset_2 + text_height + x_padding), C_background_);
#endif
    // The Area
    painter.drawText(QRect(x_padding, y_offset_1, prefix_width, text_height),
                     Qt::AlignRight | Qt::AlignBaseline, label_area);
    painter.drawText(QRect(x_padding + prefix_width, y_offset_1, value_width, text_height),
                     Qt::AlignRight | Qt::AlignBaseline, area_str);
    painter.drawText(QRect(x_padding + prefix_width + value_width, y_offset_1, value_width, text_height),
                     Qt::AlignLeft | Qt::AlignBaseline, label_AU);

    // The time
    painter.drawText(QRect(x_padding, y_offset_2, prefix_width, text_height),
                     Qt::AlignRight | Qt::AlignBaseline, label_time);
    painter.drawText(QRect(x_padding + prefix_width, y_offset_2, value_width, text_height),
                     Qt::AlignRight | Qt::AlignBaseline, days_str);
    painter.drawText(QRect(x_padding + prefix_width + value_width, y_offset_2, value_width, text_height),
                     Qt::AlignLeft | Qt::AlignBaseline, label_days);


    // Superscript 2 for AU^2
    painter.setFont(font_small);
    painter.drawText(QRect(x_padding + prefix_width + value_width + fm.width(label_AU), x_padding,
                           1.5f * fm_small.width("2"), fm_small.height()),
                     Qt::AlignLeft | Qt::AlignBaseline, "2");

    // Upload the texture
    labels_atlas_.update_tile(label_ix, label_image_.bits());

#if 0
    image_.save("sweep_label.png");
    labels_atlas_.save_atlas("sweep_label_atlas.png");
#endif
//    check_GL_error("SweepLabelAtlas::writeLabel() exit ");
  }

  /**
   *
   * Sweeps::render
   *
   */
  void render(Camera <float> &camera_){

    // Draw Sweep geometry
    glUseProgram(program_);
    glBindVertexArray(vao);

    // TODO: Cleanup
    glUniformMatrix4fv(mvHandle_, 1, GL_FALSE, camera_.mv);
    glUniformMatrix4fv(pHandle_, 1, GL_FALSE, camera_.p);

    int i = 0;

    for(const s_sweep &s : sweeps_list_){
      glUniform4fv(colorHandle_, 1, s.color);
      glDrawElements(GL_TRIANGLES, s.count, GL_UNSIGNED_INT, s.start);
      ++i;
    }

#if 0
    // Draw the entire sweep
    float color[4] = {1.0f, 1.0f, 0.0f, 1.0f};
    glUniform4fv(colorHandle_, 1, color);
    glDrawElements(GL_TRIANGLES, capacity_, GL_UNSIGNED_INT, index_);
#endif

    // Draw the labels
    labels_atlas_.bind();
    labels_billboards_.render(camera_);
    check_GL_error("Sweeps::render(Camera<float>& camera_) exit ");
  }

};


/**
 * The KeplerScene class contains most settings for the Kepler Lab simulation.
 *
 *
 */
class KeplerScene : public QOpenGLWidget {
Q_OBJECT

public:
  KeplerScene(QWidget *parent);

  /**
   *
   * Configure font sizes
   *
   * @param scale
   */
  void setInterfaceScale(float ruler_font_scale, float sweep_font_scale){
    ruler_.font_size_large = int(ruler_font_scale * ruler_.default_font_size_);
    ruler_.font_size_small = int(ruler_font_scale * ruler_.default_font_sm_size_);

    sweeps_.font_size_ = int(sweep_font_scale * sweeps_.default_font_size_);
    sweeps_.font_sm_size_ = int(sweep_font_scale * sweeps_.default_font_sm_size_);
  }

  void initializeGL();

  void cleanup();

  /**
   * Called when the window is initialized
   *
   * @param w
   * @param h
   */
  void resizeGL(int w, int h);

  /**
   *
   * @param angle_degrees
   */
  void setLaunchAngle(float angle_degrees){
#if 0
    std::cout<<"KeplerScene::setLaunchAngle "<<angle_degrees<<std::endl;
#endif
    theta_launch_ = M_PI * angle_degrees / 180.0f;

    if(!initialized_){
      return;
    }

    arrow_.set_angle(theta_launch_);
    update();
  }

  /**
   *
   * Set the length of the arrow
   *
   * @param velocity [km/s]s
   */
  void setLaunchVelocity(float velocity) {
#if 0
    std::cout<<"KeplerScene::setLaunchVelocity "<<velocity<<std::endl;
#endif

    v_launch_ = velocity;

    if(!initialized_){
      return;
    }

    arrow_.set_length(v_launch_ / v_max_);
    update();
  }

  void newOrbit();

  void runSimulation();

  void updateAnimation();

  /**
   *
   * Start a sweep from the current_element_
   *
   */
  void startSweep(){

    sweep_element_ix_ = element_ix_;

    // Create a new s_sweep starting from this element
    sweeps_.addSweep(sweep_element_ix_,
                     orbit_.elements_[element_ix_ + 1].x,
                     orbit_.elements_[element_ix_ + 1].y);
    sweep_running_ = true;
  }

  bool stopSweep(){
    sweep_running_ = false;
    sweeps_.updateSweep(sweep_element_ix_);
    return true;
  }

  void clearSweeps(){
    sweeps_.clear();
    update();
  }

  bool sweepsAvailable(){
    return sweeps_.sweep_count_ < sweeps_.max_sweep_count_;
  }

  void showCircle(bool visible){
    circle_visible_ = visible;
    update();
  }

  void showRuler(bool visible){

    // Reset ruler reuler location
    if(!ruler_visible_){
      ruler_.reset();
    }

    ruler_visible_ = visible;
    update();
  }

  /**
   * Draw the OpenGL Scene
   */
  void paintGL();

  /**
   * Event Handler
   *
   * @param event
   * @return
   */
  bool event(QEvent* event);

  /**
   *
   * Perform a hit test on the entire scene
   *
   * @param x screen coordinates
   * @param y
   * @return
   */
  bool hitTest(int x, int y);

  bool handleDrag(int x_start, int y_start, int x_end, int y_end);

  /**
   * Status functions
   */

  /**
 *
 * @return true if a simulation has been run
 */
  bool isValid(){
    return valid_;
  }

  bool isCrashed(){
    return orbit_.collision_;
  }

  bool isClosed(){
    return orbit_.closed_;
  }

  bool isCircleEnabled(){
    return circle_visible_;
  }

  bool isRulerEnabled(){
    return ruler_visible_;
  }

  float getDefaultAngle(){
    return theta_default_;
  }

  float getDefaultVelocity(){
    return v_default_;
  }

  float getVelocityMax(){
    return v_max_;
  }

signals:
  void new_orbit();

  void arrow_changed(float velocity, float angle);

protected:

  bool valid_ = false;
  bool orbit_complete_ = false;
  bool dragging_ = false;

  bool initialized_ = false;

  const static int max_cursors = 2;
  int last_cursor[max_cursors][2];

  bool animation_ = false;
  bool sweep_running_ = false;

  // Scene State
  bool arrow_visible_ = true;
  bool circle_visible_ = false;
  bool ruler_visible_ = false;

  /**
   *
   * Settings
   *
   */
  float radius_sun_ = 0.08f;
  float radius_planet_ = 0.04f;

  // Marker sizes (?)
  float radius_marker_big_ = 14.0f;
  float radius_marker_small_ = 9.0f;

  // A marker is dropped every
  int steps_per_marker_ = 7;

  // Each simulation 'tick' takes this many ms to cover.
  // (increase to slow down animation)
  int ms_per_tick_ = 16.0f;

  // Default value for the velocity. 30.0 will produce a perfect circle (c.f. struct Orbit)
  const float theta_default_ = 12.0f;
  const float v_default_ = 20.0f;

  float defaultPosition_[2] = {0.0f, 1.0f};

  const float v_max_ = 45.0f;

  // Color for markers (small and big)
  float C_markers[2][4] = {{1.0f, 1.0f, 0.0f, 1.0f},
                           {1.0f, 0.0f, 0.0f, 1.0f}};

  /**
   *  Various counters
   */
  float theta_launch_;
  float v_launch_;

  int T_prev_;

  // Current element on path
  int element_ix_;

  // The element_ix for sweeps
  int sweep_element_ix_;

  float position_[2];

  /**
   * State
   */

  /**
   * View Parameters
   */
  float near_ = -2.0f;
  float far_ = 2.0f;
  float top_ = -2.0f;
  float left_ = -2.0f;

  float center_[3] = {0.0f, 0.0f, 0.0f};
  float up_[3] = {0.0f, 1.0f, 0.0f};
  float eye_[3] = {0.0f, 0.0f, 1.0f};

  /**
   * Map out the texture units
   */
  const static int tex_unit_ruler_ = 0;
  const static int tex_unit_sweeps_ = 1;

  /**
   * Internal State
   */

  // The camera
  Camera<float> camera_;

  // The simulation
  Orbit<float> orbit_;

  /**
   * Scene Graph
   */

  // The sun
  msg::Billboard sun_;

  // The planet
  msg::Billboard planet_;

  // Program for drawing the planet
  msg::SphereProgram planetProgram_;

  // The drag handle for the ruler and the vector arrow
  msg::Billboard handle_;

  // The arrow (macro-node)
  Arrow arrow_;

  // The trail markers
  msg::Sprites markers_;
  msg::SpriteProgram markerProgram_;

  // This is the white line made by the orbit
  msg::Path orbit_path_;

  // This is the reference circle
  msg::Path circle_;

  // Program for drawing msg::Paths
  msg::PathProgram pathProgram_;

  // Sweep macro-node (contains own programs)
  Sweeps sweeps_;

  // This is the reference circle
  Ruler ruler_;

  msg::Node *hitNode_ = nullptr;

  QTime animation_timer_;
};


#endif // KEPLER_SCENE_GRAPH_H
