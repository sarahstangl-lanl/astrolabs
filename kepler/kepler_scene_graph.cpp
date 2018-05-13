/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: Contains 3D rendering code for the
 * scene. The QOpenGLwidget code.
 *
 *
 */

#include "kepler_scene_graph.h"

#include <cmath>
#include <ctime>
#include <iostream>

#include <QGestureEvent>
#include <QSurfaceFormat>
#include <QVBoxLayout>
#include <QWidget>
//#include <QtGui/QOpenGLContext>
#include <QtWidgets/QGestureEvent>

#ifndef M_PI
const double M_PI = 3.14159265359;
#endif


using namespace std;

// Modified version of this https://github.com/openscenegraph/osg/blob/master/examples/osgviewerQt/CMakeLists.txt
KeplerScene::KeplerScene(QWidget *parent)
    : QOpenGLWidget(parent),
      markers_(8096),
      orbit_path_(8096),
      circle_(256),
      sweeps_(tex_unit_sweeps_, 8096),
      ruler_(tex_unit_ruler_){

  setMouseTracking(true);
  //setFocusPolicy(Qt::ClickFocus);

  // Enable Touch
  setAttribute(Qt::WA_AcceptTouchEvents, true);
  //setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents, true);

//  grabGesture(Qt::TapGesture);
//  grabGesture(Qt::TapAndHoldGesture);
//  grabGesture(Qt::PanGesture);
//  grabGesture(Qt::PinchGesture);
//  grabGesture(Qt::SwipeGesture);
//  grabGesture(Qt::CustomGesture);

  // Setup OpenGL 3.3
  QSurfaceFormat glFormat;
  glFormat.setVersion(3, 3);
  glFormat.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(glFormat);
//  _animation_running = false;

  theta_launch_ = 0.0;
  v_launch_ = 0.0;

  // Initialize Qt
  QVBoxLayout *layout = new QVBoxLayout();
  setLayout(layout);
}

/**
 *
 * http://doc.qt.io/qt-5/qtopengl-hellogl2-glwidget-cpp.html
 *
 */
void KeplerScene::initializeGL(){

  using namespace std;

  cout << "KeplerScene::initializeGL" << endl;

#if USE_GLEW
  GLenum err = glewInit();
  if(GLEW_OK != err){
    std::cerr << "[Error] GLEW failed to initialize. " << (const char *) glewGetErrorString(err) << endl;
  }
#endif

  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClearDepth(1.0f);

  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
//  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Reference Ruler
  ruler_.init_resources();

  // Initialize the path program and array
  pathProgram_.init_resources();
  pathProgram_.build();

  // Generate reference circle
  circle_.init_resources();

  float radius = 1.0f;

  float z_circle = -0.0001f;

  float dt = float(2.0 * M_PI) / float(circle_.capacity_ - 1);
  float t = 0.0f;

  circle_.size = 0;
  for(int i = 0; i < circle_.capacity_; ++i){

    float x = radius * cos(t);
    float y = radius * sin(t);
    t += dt;

    if(!circle_.addPoint(x, y, z_circle, 0.0f, 0.0f, 0.0f)){
      break;
    }
  }

  circle_.upload();
  circle_.setup_array(pathProgram_.positionHandle_, pathProgram_.colorHandle_);
  circle_.draw_size = circle_.size;

  // The Velocity vector arrow
  arrow_.init_resources();

  // ... Planetary Bodies
  planetProgram_.init_resources();
  planetProgram_.build();

  sun_.radius = radius_sun_;
  sun_.init_resources();
  sun_.setup_array(planetProgram_.positionHandle_);

  handle_.radius = 0.25f;
  handle_.init_resources();
  handle_.setup_array(planetProgram_.positionHandle_);

//  planet_.setColor(0.5, 0.5, 1.0, 1.0);
  planet_.radius = radius_planet_;
  planet_.setPosition(position_[0], position_[1], 0);
  planet_.init_resources();
  planet_.setup_array(planetProgram_.positionHandle_);

  orbit_path_.init_resources();
  orbit_path_.setup_array(pathProgram_.positionHandle_, pathProgram_.colorHandle_);

  // ... Markers
  markerProgram_.init_resources();
  markerProgram_.build();
  markers_.init_resources();
  markers_.setup_array(markerProgram_.positionHandle_, markerProgram_.sizeHandle_, markerProgram_.colorHandle_);

  // Initialize the sweeps
  sweeps_.init_resources();

  initialized_ = true;

  newOrbit();

  check_GL_error("KeplerScene::initializeGL() exit ");
}

/**
 *
 * Free OpenGL and other resources
 *
 */
void KeplerScene::cleanup(){

  arrow_.cleanup();
  ruler_.cleanup();
  circle_.cleanup();
  handle_.cleanup();

  sun_.cleanup();
  planet_.cleanup();
  sweeps_.cleanup();
  markers_.cleanup();

  // Free the various programs
  markerProgram_.cleanup();
  pathProgram_.cleanup();
  planetProgram_.cleanup();
}

/**
 * Called When the view is resized
 *
 * @param w
 * @param h
 */
void KeplerScene::resizeGL(int w, int h){

#if 0
  float fov = 30.0f;
  float near = 1.0f;
  float far = 10000.0f;
  camera_.init_perspective(w, h, fov, near, far);
#else
  float aspect = float(w) / float(h);

  float top = top_;
  float left = left_;

  if(aspect <= 1.0){
    top /= aspect;
  }else{
    left *= aspect;
  }
  camera_.init_orthographic(w, h, left, top, near_, far_);
#endif

  camera_.init_model_view();
  camera_.look_at(center_, eye_, up_);

  glViewport(0, 0, w, h);
  update();
}

/**
 *
 *  Reset the scene to the default
 *
 */
void KeplerScene::newOrbit(){
  /**
   * Coordinate system is setup so that the Sun is at the center of the screen which is (0, 0).
   * Y+ is up, X+ is to the right
   *
   */

//  std::cout<<"KeplerScene::newOrbit"<<std::endl;
  animation_timer_.start();

  valid_ = false;
  animation_ = false;
  orbit_complete_ = false;

  element_ix_ = 0;
  sweep_element_ix_ = 0;

  // Return planet to start position
  planet_.setPosition(defaultPosition_[0], defaultPosition_[1], 0);

  // Adjust arrow
  arrow_.set_angle(theta_launch_);
  arrow_.set_length(v_launch_ / v_max_);

  // Delete the sweeps
  sweeps_.clear();

  // Remove the orbit trail
  markers_.clear();

  // And clear the path
  orbit_path_.clear();

  arrow_visible_ = true;
  ruler_visible_ = false;
  circle_visible_ = false;

  sweep_running_ = false;
  animation_ = false;

  emit new_orbit();

  update();
}

/**
 * KeplerScene::runSimulation
 *
 *   Run the orbital simulation and construct the geometry that represents it
 *
 *
 */
void KeplerScene::runSimulation(){

  arrow_visible_ = false;

  // Compute the orbit
  orbit_.calculate_orbit(theta_launch_, v_launch_);
#if 0
  std::cout<<"KeplerScene::runSimulation: Computed Orbit"<<std::endl;
  std::cout<<"KeplerScene::runSimulation: Closed orbit: "<<orbit_.closed_<<std::endl;
  std::cout<<"KeplerScene::runSimulation: Max time is "<<orbit_.t_max_<<std::endl;
#endif

  // Clear old data
  orbit_path_.size = 0;
  markers_.size = 0;
  sweeps_.clear();

  for(int i = 0; i < orbit_.element_count_; ++i){

    // Build the orbit path
    if(!orbit_path_.addPoint(orbit_.elements_[i].x, orbit_.elements_[i].y, 0.0,
                             orbit_.elements_[i].v_x, orbit_.elements_[i].v_y, 0.0)){
      cerr << "KeplerScene::runSimulation : Out of orbital path capacity " << orbit_path_.capacity_ << endl;
      break;
    }

    // Build the s_sweep data
    if(!sweeps_.addPoint(orbit_.elements_[i].x, orbit_.elements_[i].y, 0.0)){
      cerr << "KeplerScene::runSimulation : Out of sweep capacity " << sweeps_.capacity_ << endl;
      break;
    }

    // Create the markers
    if(i % steps_per_marker_ == 0){

      int color_ix = 0;
      float radius = radius_marker_small_;

      if((markers_.size % 5 == 0)){
        radius = radius_marker_big_;
        color_ix = 1;
      }

      if(!markers_.addPoint(orbit_.elements_[i].x, orbit_.elements_[i].y, 0.0f, radius,
                            C_markers[color_ix][0], C_markers[color_ix][1],
                            C_markers[color_ix][2], C_markers[color_ix][3])){
        cerr << "KeplerScene::runSimulation : Out of marker capacity " << markers_.capacity_ << endl;
        break;
      }
    }
  }

  orbit_path_.upload();
  markers_.upload();
  sweeps_.buildUpload(orbit_.closed_);

  glFlush();
  glFinish();

#if 0
  std::cout<<"!!! Total Markers "<<markers_.size<<std::endl;
  std::cout<<"!!! Orbit Size "<<orbit_path_.size<<std::endl;
  std::cout<<"!!! Orbit Element Count "<<orbit_.element_count_<<std::endl;
#endif

  valid_ = true;

  // Start the animation
  animation_ = true;
  animation_timer_.start();
  T_prev_ = animation_timer_.elapsed();

  update();
}

/**
 *
 * This method fires every animation tick (16ms for 60fps) and updates
 * animations
 *
 */
void KeplerScene::updateAnimation(){

  using namespace std;

  if(!animation_){
    return;
  }

  int T_cur = animation_timer_.elapsed();

  // Number of millis elapsed between last frame and now
  float delta_time = (T_cur - T_prev_);
  T_prev_ = T_cur;

//  int ticks = delta_time / ms_per_tick;
//  float excess = delta_time - ms_per_tick * ticks;

  /**
   *
   * Each timestep we click over a specific number of elements
   *
   */
  int d_ix = int(delta_time / ms_per_tick_);

  // Change planet position
  planet_.setPosition(orbit_.elements_[element_ix_ + 1].x,
                      orbit_.elements_[element_ix_ + 1].y,
                      0.0f);

  element_ix_ += d_ix;

  if((orbit_.element_count_ > 0) && (element_ix_ >= orbit_.element_count_)){
    orbit_complete_ = true;
    element_ix_ = element_ix_ % orbit_.element_count_;
  }

  if(orbit_complete_){
    markers_.draw_size = markers_.size;
    orbit_path_.draw_size = orbit_path_.size;

    if(!orbit_.closed_){
      animation_ = false;
    }

    if(orbit_.collision_){
      planet_.setPosition(0.0f, 0.0f, -0.1f);
    }

  }else{
    markers_.draw_size = element_ix_ / steps_per_marker_;
    orbit_path_.draw_size = element_ix_;
  }

  // If a s_sweep is currently running
  if(sweep_running_){
    // End sweeps after about 1.5 orbits
    if(sweeps_.updateSweep(sweep_element_ix_) > 1.5 * orbit_.element_count_){
      sweep_running_ = false;
    }else{
      sweep_element_ix_ += d_ix;
    }
  }

  update();
}


/**
 *
 * Draw the actual scene. This is done back to front.
 *
 */
void KeplerScene::paintGL(){

//  std::cout<<"KeplerScene::paintGL"<<std::endl;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//  glEnable(GL_CULL_FACE);

  // Draw

  // ... paths
  glUseProgram(pathProgram_.program_);

  // The Model View and Projection matrices. Each `node` has its own MV
  // so we supply the p matrix separately... I guess. Can easily do it offline too.
  glUniformMatrix4fv(pathProgram_.mvpHandle_, 1, GL_FALSE, camera_.mvp);

  // Orbit path
  float orbit_color_[4] = {1.0f, 1.0f, 1.0f, 1.0f};
  glUniform4fv(pathProgram_.colorHandle_, 1, orbit_color_);
  orbit_path_.render(camera_);

  // Reference Circle
  if(circle_visible_){
    float circle_color_[4] = {0.5f, 1.0f, 0.5f, 1.0f};
    glUniform4fv(pathProgram_.colorHandle_, 1, circle_color_);
    circle_.render(camera_);
  }

  // ... the Sweeps (own program)
  sweeps_.render(camera_);

  // ... the Markers
  glUseProgram(markerProgram_.program_);
  glUniformMatrix4fv(markerProgram_.mvpHandle_, 1, GL_FALSE, camera_.mvp);
  markers_.render(camera_);

  /**
   * Drag Markers
   */
  if(arrow_visible_ || ruler_visible_){
    glUseProgram(planetProgram_.program_);
  }

  // For the arrow
  if(arrow_visible_){
    // HACK: Using the sun blob and planet program for the drag marker
    float blob_color[4] = {1.0f, 1.0, 1.0f, 0.5f};
    float blob_offset[4] = {arrow_.x_tip_, arrow_.y_tip_, 0.0f, 0.0f};
    float blob_radius = 0.04f;

    glUniform4fv(planetProgram_.colorHandle_, 1, blob_color);
    glUniform3fv(planetProgram_.offsetHandle_, 1, blob_offset);
    glUniform1f(planetProgram_.radiusHandle_, blob_radius);
    handle_.render(camera_);
  }

  // and for the ruler
  if(ruler_visible_){

    float blob_color[4] = {1.0f, 1.0, 1.0f, 0.5f};

    glUniform4fv(planetProgram_.colorHandle_, 1, blob_color);
    glUniform1f(planetProgram_.radiusHandle_, ruler_.handle_radius);

    glUniform3fv(planetProgram_.offsetHandle_, 1, &ruler_.handle_position[0][0]);
    handle_.render(camera_);

    glUniform3fv(planetProgram_.offsetHandle_, 1, &ruler_.handle_position[1][0]);
    handle_.render(camera_);
  }

  // Draw arrow
  if(arrow_visible_){
    // Arrow has own program
    arrow_.render(camera_);
  }

  // Draw the planet and the sun
  glUseProgram(planetProgram_.program_);

  // The Model View and Projection matrices. Each `node` has its own MV
  // so we supply the p matrix separately... I guess. Can easily do it offline too.
  glUniformMatrix4fv(planetProgram_.mvpHandle_, 1, GL_FALSE, camera_.mvp);
//  glUniformMatrix4fv(planetProgram_.pHandle_, 1, GL_FALSE, camera_.p);

  // ... Planet
  float planet_color_[4] = {0.5f, 0.5f, 1.0f, 1.0f};
  glUniform4fv(planetProgram_.colorHandle_, 1, planet_color_);
  glUniform3fv(planetProgram_.offsetHandle_, 1, planet_.position);
  glUniform1f(planetProgram_.radiusHandle_, planet_.radius);
  planet_.render(camera_);

  // ... sun
  float sun_color_[4] = {1.0f, 1.0, 0.0f, 1.0f};
  glUniform4fv(planetProgram_.colorHandle_, 1, sun_color_);
  glUniform3fv(planetProgram_.offsetHandle_, 1, sun_.position);
  glUniform1f(planetProgram_.radiusHandle_, sun_.radius);
  sun_.render(camera_);

  // Reference Ruler (has own program)
  if(ruler_visible_){
    ruler_.render(camera_);
  }

  check_GL_error("KeplerScene::paintGL() exit");
}

/**
 * Generic Event handler but we only capture Mouse Press and Move
 *
 * @param event
 * @return
 */
bool KeplerScene::event(QEvent *event){
  const QMouseEvent *e = static_cast<QMouseEvent *>(event);

  if((event->type() == QEvent::MouseMove) && dragging_){
    return handleDrag(last_cursor[0][0], last_cursor[0][1], e->x(), e->y());
  }else if(event->type() == QEvent::MouseButtonPress){
    // Check if something intersects
    dragging_ = hitTest(e->x(), e->y());
    return true;
  }else if(event->type() == QEvent::MouseButtonRelease){
    dragging_ = false;
    hitNode_ = nullptr;
    return true;
  }

  return QWidget::event(event);
}

/**
 * Check if the click interfaces
 *
 * @param x
 * @param y
 * @return
 */
bool KeplerScene::hitTest(int x, int y){

  using namespace std;

  float world_cs[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  camera_.unproject(x, y, -1.0f, world_cs);

#if 0
  std::cout<<" ######## ##### ##### #### "<<endl;
  std::cout << "hitTest Mouse = "<<x<<", "<<y<<std::endl;
  std::cout << "hitTest Screen Size: "<<width()<<", "<<height()<<std::endl;
  std::cout << "hitTest world_cs = " << world_cs[0] << ", " << world_cs[1] << ", " << world_cs[2] << ", " << world_cs[3] <<std::endl;
#endif

  if(arrow_.hitTest(world_cs[0], world_cs[1])){
    hitNode_ = &arrow_;
  }else if(ruler_.hitTest(world_cs[0], world_cs[1])){
    hitNode_ = &ruler_;
  }else{
    hitNode_ = nullptr;
    return false;
  }

  last_cursor[0][0] = x;
  last_cursor[0][1] = y;
  return true;
}

/**
 *
 * @param x_start
 * @param y_start
 * @param x_end
 * @param y_end
 * @return
 */
bool KeplerScene::handleDrag(int x_i, int y_i, int x_f, int y_f){
//  std::cout << "handleDrag x = "<<x_i<<" ... "<<x_f<<std::endl
//            << " x = "<<y_i<<" ... "<<y_f<<std::endl;

  if(hitNode_ == nullptr){
    return false;
  }

  float world_i[4] = {0.0f, 0.0f, 0.0f, 1.0f};
  float world_f[4] = {0.0f, 0.0f, 0.0f, 1.0f};

  camera_.unproject(x_i, y_i, -1.0f, world_i);
  camera_.unproject(x_f, y_f, -1.0f, world_f);

  if(hitNode_->handleDrag(world_i[0], world_i[1], world_f[0], world_f[1])){
    last_cursor[0][0] = x_f;
    last_cursor[0][1] = y_f;

    if(hitNode_ == &arrow_){
      theta_launch_ = arrow_.theta_;
      v_launch_ = arrow_.length_ * v_max_;
      emit arrow_changed(v_launch_, theta_launch_);
    }

    update();
    return true;
  };

  return false;
}