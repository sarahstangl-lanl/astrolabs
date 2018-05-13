/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: Dark Matter lab, velocity measurement lab
 * 3D scene code.
 *
 */

#include "dm_scene_graph.h"
#include "dm_gui.h"

#include <cmath>
#include <iostream>
#include <random>

#include <QGestureEvent>
#include <QImage>
#include <QPainter>
#include <QTime>
#include <QTimer>
#include <QTimerEvent>
#include <QVBoxLayout>
#include <QWidget>


using namespace std;

DarkMatterScene::DarkMatterScene(QWidget *parent)
    : QOpenGLWidget(parent),
      reticule_(tex_unit_reticule_label_),
      spiral_galaxy_(tex_unit_galaxy_),
      h2_region_(10),
      star_cluster_(20),
      cluster_background_(tex_unit_cluster_, 1024, 1024, 1),
      lensing_(tex_unit_lensing_, tex_unit_legend_),
      dm_sprites_(N_dm_sprites_){

  setMouseTracking(true);

  // Setup OpenGL 3.3
  QSurfaceFormat glFormat;
  glFormat.setVersion(3, 3);
  glFormat.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(glFormat);

  // Initialize Qt
  QVBoxLayout *layout = new QVBoxLayout();
  setLayout(layout);

  startTimer(20);

  /**
   * Construct Scenes
   */
  // TODO: Starting positions?


  // The selection box
  reticule_.setPosition(0, 0, 0);

  float R_scale = 1.0f / 1e5f;

  float R_sun = 1e5;
  float R_mercury = 4000.0f;
  float R_venus = 6051.0f;
  float R_earth = 6378.0f;
  float R_mars = 5396.0f;

  // Inner solar system
  sun_.radius = 0.1 * R_sun * R_scale;
  // sun_.rotation_ = 601.2;

  mercury_.radius = R_mercury * R_scale;
  mercury_.obliquity = 2.11f;
  //  mercury_.rotation = 1407.5f;

  venus_.radius = R_venus * R_scale;
  venus_.obliquity = 177.3f;
  //  venus_.rotation = 5832.0f;

  earth_.radius = R_earth * R_scale;
  earth_.obliquity = 23.5f;
  //  mars_.rotation = 24.0f;

  mars_.radius = R_mars * R_scale;
  mars_.obliquity = 25.19f;
  //  mars_.rotation = 24.6f;

  /**
   *
   * Note: We have a default camera view which is Y is up and looking down negative Z.
   *
   * All billboards are oriented so that they are in the XY plane. When setting the inclination
   * we compute a rotation matrix or quaternion from default camera orientation to the desired
   * inclination. Inclination of 90 is edge on.
   *
   */

  // Setup the scenes
  scenes_[0].name = "Part 1: Solar System";
  scenes_[0].setOrbitInclination((float) (0.5f * M_PI), (float) (0.025f * M_PI));
  scenes_[0].setFinalEyeLocation(0.0f, 0.0f, 3.0f);
  scenes_[0].updateInclination(0);

//  scenes_[0].add_body(new OrbitingObject(&sun_, "Sun", 601.2, 0.0f, 0.0f, 0.0f));

#if 0
  // Node, Name, R_orbital (AU), V_orbial (km/s), Inclination, Eccentricity
  scenes_[0].add_body(new OrbitingObject(&mercury_, "Mercury", 0.387f, 47.87f, 7.01f, 0.20563f));
  scenes_[0].add_body(new OrbitingObject(&venus_, "Venus", 0.723f, 35.02f, 3.39f, 0.006772f));
  scenes_[0].add_body(new OrbitingObject(&earth_, "Earth", 1.0f, 29.78f, 0.0f, 0.0167f));
  scenes_[0].add_body(new OrbitingObject(&mars_, "Mars", 1.5f, 24.077f, 1.85f, 0.0934f));
#else
  // Node, Name, R_orbital (AU), V_orbial (km/s), Inclination, Eccentricity
  scenes_[0].add_body(new OrbitingObject(&mercury_, "Mercury", 0.387f, 47.87f, 0.0f, 0.20563f));
  scenes_[0].add_body(new OrbitingObject(&venus_, "Venus", 0.723f, 35.02f, 0.0f, 0.006772f));
  scenes_[0].add_body(new OrbitingObject(&earth_, "Earth", 1.0f, 29.78f, 0.0f, 0.0167f));
  scenes_[0].add_body(new OrbitingObject(&mars_, "Mars", 1.5f, 24.077f, 0.0f, 0.0934f));
#endif

  scenes_[1].name = "Part 2: Spiral Galaxy";
  scenes_[1].setOrbitInclination((float) (0.5f * M_PI), (float) (0.025f * M_PI));
  scenes_[1]._background_texture = "textures/spiral_galaxy.jpg";
  scenes_[1].setFinalEyeLocation(0.0f, 0.0f, 6.0f);
  scenes_[1].updateInclination(0);

  // HACK!
  float r_scale = 1.0f / 10.0f;

  scenes_[1].add_body(new OrbitingObject(&dark_cloud_, dark_cloud_.name_, 3.0f * r_scale, 150.0f, 0.0f, 0.0f));
  scenes_[1].add_body(new OrbitingObject(&h2_region_, h2_region_.name_, 10.1f * r_scale, 210.0f, 0.0f, 0.0f));
  scenes_[1].add_body(new OrbitingObject(&bipolar_nebula_, bipolar_nebula_.name_, 16.3f * r_scale, 250.0f, 0.0f, 0.0f));
  scenes_[1].add_body(new OrbitingObject(&star_cluster_, star_cluster_.name_, 20.7f * r_scale, 230.0f, 0.0f, 0.0f));
  scenes_[1].add_body(
      new OrbitingObject(&planetary_nebula_, planetary_nebula_.name_, 26.5f * r_scale, 230.0f, 0.0f, 0.0f));

  scenes_[2].name = "Part 3: Galaxy Cluster";
  scenes_[2].setOrbitInclination(0.0f, 0.0f);
  scenes_[2].setFinalEyeLocation(0.0f, 0.1f, 4.0f);
  scenes_[2].updateInclination(0);

  r_scale = 1.0f / 200.0f;

#if 1
  // Node, Name, Distance (kpc, uknown units)
  scenes_[2].add_body(new OrbitingObject(&cluster_galaxy_[0], "Galaxy 1", 18.6f * r_scale, 1905.7f, 0.0f, 0.0f));
  scenes_[2].add_body(new OrbitingObject(&cluster_galaxy_[1], "Galaxy 2", 78 * r_scale, 2053.0f, 270.0f, 0.0f));
  scenes_[2].add_body(new OrbitingObject(&cluster_galaxy_[2], "Galaxy 3", 228 * r_scale, 2100.6f, 160.0f, 0.0f));
  scenes_[2].add_body(new OrbitingObject(&cluster_galaxy_[3], "Galaxy 4", 110 * r_scale, 2024.0f, 15.0f, 0.0f));
  scenes_[2].add_body(new OrbitingObject(&cluster_galaxy_[4], "Galaxy 5", 168 * r_scale, 2081.0f, 45.0f, 0.0f));
#else
  scenes_[2].add_body(new OrbitingObject(&galaxies_path_[0], "Galaxy 1", 78, 2053.0, 270, 0.0f));
  scenes_[2].add_body(new OrbitingObject(&galaxies_path_[1], "Galaxy 2", 18.6, 1905.7, 0, 0.0f));
  scenes_[2].add_body(new OrbitingObject(&galaxies_path_[2], "Galaxy 3", 228, 2100.6, 160, 0.0f));
  scenes_[2].add_body(new OrbitingObject(&galaxies_path_[3], "Galaxy 4", 110, 2024.0, 15, 0.0f));
  scenes_[2].add_body(new OrbitingObject(&galaxies_path_[4], "Galaxy 5", 168, 2081.0, 45, 0.0f));
#endif

  // Start the animation from the beginning
  animation_ = true;
  animation_time_.start();
  T_prev_ = animation_time_.elapsed();

}

/**
 *
 * http://doc.qt.io/qt-5/qtopengl-hellogl2-glwidget-cpp.html
 *
 */
void DarkMatterScene::initializeGL(){

//  std::cout << "DarkMatterScene::initializeGL" << std::endl;

#if USE_GLEW
  GLenum err = glewInit();
  if(GLEW_OK != err){
    std::cerr << "[Error] GLEW failed to initialize. " << (const char *) glewGetErrorString(err) << endl;
  }
#endif

  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
  glClearDepth(1.0f);

  glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
  glEnable(GL_PROGRAM_POINT_SIZE);
  glEnable(GL_BLEND);
  glEnable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  /**
   * Initialize the scenegraph resources
   */

  // Measurement reticule
  reticule_.init_resources();
  reticule_.updatePosition(0.0, 0.0);

  // Solar System
  sun_.init_from_file(tex_unit_sun_, "assets/textures/sunmap.jpg");
  sun_.setPosition(0.0f, 0.0f, 0.0f);

  mercury_.init_from_file(tex_unit_mercury_, "assets/textures/mercurymap.jpg");
  venus_.init_from_file(tex_unit_venus_, "assets/textures/venusmap.jpg");
  earth_.init_from_file(tex_unit_earth_, "assets/textures/earthmap1k.jpg");
  mars_.init_from_file(tex_unit_mars_, "assets/textures/mars_1k_color.jpg");

  // Spiral Galaxy
  spiral_galaxy_.init_resources();

  dark_cloud_.init_resources();
  h2_region_.init_resources();
  bipolar_nebula_.init_resources();
  star_cluster_.init_resources();
  planetary_nebula_.init_resources();

  // Scene 3
  for(int i = 0; i < galaxies_ct_; ++i){
    cluster_galaxy_[i].init_resources();
  }

  cluster_background_.alpha_ = 1.0f;
  cluster_background_.init_resources();
  cluster_background_.bind();

  const char *cluster_image_path_ = "assets/backgrounds/Abell_370.jpg";
  QImageReader reader(cluster_image_path_);
  const QImage cluster_image = reader.read();

  if(cluster_image.isNull()){
    cerr << "Cannot load: " << cluster_image_path_
         << ", " << reader.errorString().toStdString() << endl;
  }
//    galaxy_img.save("galaxy_img.png");
  QImage image(cluster_background_.tex_width_,
               cluster_background_.tex_height_,
               QImage::Format_RGBA8888);
  image.fill(qRgba(0, 0, 0, 0));

  QPainter painter(&image);
  painter.drawImage(image.rect(), cluster_image);
  cluster_background_.upload_slice(0, image.bits());

  cout << " Loaded file " << cluster_image_path_
       << "  ( " << cluster_image.width() << " x " << cluster_image.height() << " )"
       << endl;

  // Init the lensing component
  lensing_.init_resources();

  /**
   * Initialize Dark Matter Sprites
   */

  // Dark Matter sprites
  dm_sprites_program_.init_resources();
  dm_sprites_program_.build();
  dm_sprites_.init_resources();
  dm_sprites_.setup_array(dm_sprites_program_.positionHandle_,
                          dm_sprites_program_.sizeHandle_,
                          dm_sprites_program_.colorHandle_);

  // Add random locations to a 2D disk.
  int random_seed_ = 1341;
  std::mt19937 generator(random_seed_);

  std::uniform_real_distribution <> rand_r(0.0f, 1.0f);
  std::uniform_real_distribution <> rand_t(0.0f, 2.0f * M_PI);

  float z_offset = -0.1f;

  for(int i = 0; i < N_dm_sprites_; ++i){

    // We are not using disk-point picking because we want particles to be bunched up.
    float r = (float) rand_r(generator);
    float t = (float) rand_t(generator);

    float x = r * cos(t);
    float y = r * sin(t);

    dm_sprites_.addPoint(x, y, z_offset, R_dm_sprites_, C_dm_sprites_[0],
                         C_dm_sprites_[1], C_dm_sprites_[2], C_dm_sprites_[3]);
  }

  dm_sprites_.upload();
  setLensMass(0.0f);

  /**
   * Create all the paths
   */
  // Path program
  pathProgram_.init_resources();
  pathProgram_.build();

  for(int i = 0; i < 3; ++i){
    for(OrbitingObject *object :  scenes_[i].bodies_){
      object->path.init_resources();
      object->path.setup_array(pathProgram_.positionHandle_, pathProgram_.colorHandle_);
      object->path.updateAttachedPositions(0.0);
    }
  }
}

/**
 *
 * Called to free resources
 *
 */
void DarkMatterScene::cleanup(){

  cout << "DarkMatterScene::cleanup() " << endl;

  reticule_.cleanup();

  // Cleanup Solar System and Spiral Galaxy
  for(int i = 0; i < 2; ++i){
    for(OrbitingObject *object :  scenes_[i].bodies_){

      // Galaxy cluster uses a billboard set
      if(i < 2){
        object->node->cleanup();
      }
      object->path.cleanup();
    }
  }

  spiral_galaxy_.cleanup();

  // Clean up the galaxy cluster scene separately
  for(int i = 0; i < galaxies_ct_; ++i){
    cluster_galaxy_[i].cleanup();
  }

//  // Paths
//  for(int i = 0; i < galaxy_ct_; ++i){
//    galaxies_path_[i].cleanup();
//  }

  dm_sprites_.cleanup();
  dm_sprites_program_.cleanup();

  pathProgram_.cleanup();
}

/**
 * Called When the view is resized
 *
 * @param w
 * @param h
 */
void DarkMatterScene::resizeGL(int w, int h){

  using namespace std;

#if 0
  cout << "DarkMatterScene::resizeGL" << endl;
#endif

  // Camera for screen-space elements
  float aspect = float(w) / float(h);

  float top = top_;
  float left = left_;

  if(aspect <= 1.0){
    top /= aspect;
  }else{
    left *= aspect;
  }


#if 1
  // Each scene has its own camera
  for(auto &&scene :  scenes_){
    // Normal Perspective Camera
    scene.camera_.init_perspective(w, h, fov_, near_, far_);
    scene.camera_.init_model_view();
    scene.camera_.look_at(center_, scene.eye_, scene.up_);
  }
#else
  // Each scene has its own camera
  for(auto &&scene :  scenes_){
    // Normal Perspective Camera
    scene.camera_.init_orthographic(w, h, left, top, near_, far_);
    scene.camera_.init_model_view();
    scene.camera_.look_at(center_, scene.eye_, scene.up_);
  }
#endif

  cluster_background_.setScaleFromAspect(aspect);
//  spiral_galaxy_.setScaleFromAspect(aspect);

  screen_camera_.init_orthographic(w, h, left, top, near_, far_);
  screen_camera_.init_model_view();
  screen_camera_.look_at(center_, screen_eye_, screen_up_);

  glViewport(0, 0, w, h);

  // Reset transforms, etc.
  updateAnimation();
}

/**
 *
 * Draw the actual scene. This is done back to front.
 *
 */
void DarkMatterScene::paintGL(){

#if 0
  std::cout<<"DarkMatterScene::paintGL "<<scene_current_<<std::endl;
#endif

  auto &camera = scenes_[scene_current_].camera_;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);

  // Render background image first
  if(scene_current_ == 2){
    cluster_background_.render(screen_camera_);
  }

  if((scene_current_ == 2) && (lensing_enabled_)){
    // Draw the Dark Matter sprites
    glUseProgram(dm_sprites_program_.program_);
    glUniformMatrix4fv(dm_sprites_program_.mvpHandle_, 1, GL_FALSE, screen_camera_.mvp);
    dm_sprites_.render(camera);
  }

  // Background image for the galaxy
  if(scene_current_ == 1){
    spiral_galaxy_.render(camera);
  }

  glEnable(GL_DEPTH_TEST);

  /**
   * Draw the paths
   */
  glUseProgram(pathProgram_.program_);
  glUniformMatrix4fv(pathProgram_.mvpHandle_, 1, GL_FALSE, camera.mvp);

  if(!lensing_enabled_) {
    for (OrbitingObject *object :  scenes_[scene_current_].bodies_) {
      object->path.render(camera);
    }
  }


#if 0
  // Render each scene, we can potentially put all these
  // nodes in an array and then loop over them...

  if(scene_current_ == 0){
    sun_.render(camera_);
  }else if(scene_current_ == 2){
    cluster_background_.render(camera_);
    galaxies_.render(camera_);

    if(lensing_enabled_){
      // The gravitational lens
      lensing_.render(camera_);
    }
  }

  /**
   * Draw the scene objects
   */
  // 0 - Solar system, 1 - Galaxy, 2 - Cluster
  for(OrbitingObject *object :  scenes_[scene_current_].bodies_){
    object->node_->render(camera_);
  }
#else
  if((scene_current_ < 3) && (!lensing_enabled_)){

    if(scene_current_ == 0){
      sun_.render(camera);
    }

    // 0 - Solar system, 1 - Galaxy, 2 - Cluster
    for(OrbitingObject *object :  scenes_[scene_current_].bodies_){
      object->node->render(camera);
    }

  }


  /**
   * Draw Lensing simulation
   */
  if(scene_current_ == 2){
//    galaxies_.render(camera);

    if(lensing_enabled_){
      glDisable(GL_DEPTH_TEST);

      // The gravitational lens
      lensing_.render(screen_camera_);

      glEnable(GL_DEPTH_TEST);
    }
  }
#endif

/**
   * Measurement reticule is last
   */
  if(!lensing_enabled_){
    // The gravitational lens
    reticule_.render(screen_camera_);
  }

  check_GL_error("DarkMatterScene::paintGL() exit");
}

/**
 *
 * This routine drives all of the animations. It looks up the position
 * of each object from their animation path. We can do something fancy
 * later.
 *
 */
void DarkMatterScene::updateAnimation(){

  // TODO: Update all OrbitingObjects in the current scene

  using namespace std;

  if(!animation_){
    return;
  }

  int T_cur = animation_time_.elapsed();

  float delta_time = (T_cur - T_prev_);

//  int ticks = delta_time / ms_per_tick;
//  float excess = delta_time - ms_per_tick * ticks;

  // Number of millis elapsed
  //float delta_time = (T_cur - T_prev_);
  T_prev_ = T_cur;

//  cout<<"delta-t is "<<delta_time<<"T_cur_ "<<T_cur<<endl;

  /**
   *
   * Update tilt animations
   *
   */

  if(scenes_[scene_current_].updateInclination(delta_time * tick_per_ms_)){
    scenes_[scene_current_].camera_.init_model_view();
    scenes_[scene_current_].camera_.look_at(center_, scenes_[scene_current_].eye_,
                                            scenes_[scene_current_].up_);

    if(scene_current_ == 0){

      sun_.inclination_ = scenes_[scene_current_].inclination;
      mercury_.inclination_ = scenes_[scene_current_].inclination;
      venus_.inclination_ = scenes_[scene_current_].inclination;
      earth_.inclination_ = scenes_[scene_current_].inclination;
      mars_.inclination_ = scenes_[scene_current_].inclination;

    }else if(scene_current_ == 1){
      spiral_galaxy_.inclination_ = scenes_[scene_current_].inclination;

      h2_region_.envelope_.inclination_ = scenes_[scene_current_].inclination;
      bipolar_nebula_.inclination_ = scenes_[scene_current_].inclination;
      dark_cloud_.inclination_ = scenes_[scene_current_].inclination;
      planetary_nebula_.inclination_ = scenes_[scene_current_].inclination;
    }

  }

  /**
   *
   * Update path animations
   *
   */

  for(OrbitingObject *object :  scenes_[scene_current_].bodies_){
    object->path.updateAttachedPositions(delta_time * tick_per_ms_);

    // Update child node positions
    object->node->updateChildren();
  }

  // Sort by view order
  scenes_[scene_current_].sort();

  update();
}

/**
 *
 * Handle mouse and touch events
 *
 * @param event
 * @return
 */
bool DarkMatterScene::event(QEvent *event){
//  std::cout<<"event "<<event->type()<<endl;

  float world_cs[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  /*
   * Handle Gestures
   */
  if(event->type() == QEvent::MouseButtonPress){
    const QMouseEvent *e = static_cast<QMouseEvent *>(event);
//    std::cout << "MouseButtonPress at " << e->x() << ", " << e->y() << endl;

    screen_camera_.unproject(e->x(), e->y(), -1.0f, world_cs);
    mouse_.down = true;
    mouse_.dragged = false;
    mouse_.dx = 0;
    mouse_.dy = 0;
    mouse_.x_last = e->x();
    mouse_.y_last = e->y();

    mouse_.selected = reticule_.hitTest(world_cs[0], world_cs[1]);

    if(mouse_.selected){
      takeMeasurement();
    }else{
      reticule_.dot_active_ = false;
      // TODO: Why is it 0.36f and not 0.50f?
      reticule_.updatePosition(world_cs[0], world_cs[1] - 0.36f * reticule_.reticule_.quad_height_);
//      reticule_.showObjectLabel(false);
      takeMeasurement();
    }

    return true;
  }else if(event->type() == QEvent::MouseButtonRelease){
    mouse_.selected = false;
    mouse_.down = false;
    return true;
  }else if(event->type() == QEvent::MouseMove){
    const QMouseEvent *e = static_cast<QMouseEvent *>(event);

    if(mouse_.selected){

      float dx = mouse_.x_last - e->x();
      float dy = e->y() - mouse_.y_last;

      mouse_.dx += dx;
      mouse_.dy += dy;

      float world_i[4] = {0.0f, 0.0f, 0.0f, 1.0f};
      float world_f[4] = {0.0f, 0.0f, 0.0f, 1.0f};

      screen_camera_.unproject(mouse_.x_last, mouse_.y_last, -1.0f, world_i);
      screen_camera_.unproject(e->x(), e->y(), -1.0f, world_f);

      reticule_.handleDrag(world_i[0], world_i[1], world_f[0], world_f[1]);

      float dist_sq = dx * dx + dy * dy;

      if(dist_sq > 0){
        mouse_.dragged = true;

        // Check if we are measuring anything new
        takeMeasurement();

        // Render
        update();
      }

      mouse_.x_last = e->x();
      mouse_.y_last = e->y();
    }

    return true;
  }

  return QWidget::event(event);
}


/**
 *
 * Collect data under the reticule.
 *
 */
void DarkMatterScene::takeMeasurement(){

  float R_pick_ = 0.03f;

  int pick_ix = -1;
  float r_min = 0.0003f;

  // Project the center of the reticule to the screen
  float pick_screen[4] = {0.0f, 0.0f, 0.0f, 0.0f};
  vec4_by_mat4x4(screen_camera_.mvp, reticule_.position, pick_screen);

  // The intersection point for the hit orbit.
  float hit_point[3];
  float hit_velocity[3];

  float x = pick_screen[0];
  float y = pick_screen[1];

  auto &camera = scenes_[scene_current_].camera_;

  reticule_.showObjectLabel(true);
  reticule_.updateObjectLabel(nullptr);

//  std::cout<<"DarkMatterScene::takeMeasurement "<<x<<", "<<y<<std::endl;

  int body_ix = 0;
  int element_ix = 0;

  for(OrbitingObject *o :  scenes_[scene_current_].bodies_){

    float r_sq = o->path.hitTest(x, y, R_pick_, scenes_[scene_current_].camera_,
                                  &element_ix, hit_point, hit_velocity);

    if(r_sq < r_min){
      r_min = r_sq;
      pick_ix = body_ix;
      reticule_.updateObjectLabel(o->name.c_str());
      reticule_.showObjectLabel(true);
    }
#if 0
    std::cout << " DarkMatterScene::hitTest Picking " << o->_name
              << " body_ix " << body_ix << " pick is " << pick_ix
              << " at a distance of " << r_sq << std::endl;
#endif

    ++body_ix;
  }


  if(pick_ix >= 0){

    // Compute the projected velocity by taking the dot product of the view vector
    // and the
    float v_projected = 0.0f;
    for(int i = 0; i < 3; ++i){
      v_projected += hit_velocity[i] * camera.eye_world[i];
    }

#if 0
    std::cout << " eye world "
              << std::setw(12) << camera.eye_world[0]
              << std::setw(12) << camera.eye_world[1]
              << std::setw(12) << camera.eye_world[2]
              << std::setw(12) << hit_velocity[0]
              << std::setw(12) << hit_velocity[1]
              << std::setw(12) << hit_velocity[2]
              << std::endl;
#endif
    // Adjust for projection effects due to the `illustrative` inclination for first two scenes
    if(scene_current_ == 0){


#if 1
      float v_scaling = 1.0f / cos(scenes_[scene_current_].inclination);
#else
      // This is hard coded to get Earth's velocity to come out correctly...
      float v_scaling = 1.4;
#endif

      v_projected *= v_scaling;
#if 0
      std::cout<<" current scene "<<scene_current_<<" inclination "<<scenes_[scene_current_].inclination<<std::endl;
      std::cout<<" v "<<v_projected<<", scaling "<<v_scaling <<", scaled v "<<v_projected * v_scaling<<std::endl;
#endif
    }

//    std::cout<<"v_projected "<<v_projected<<std::endl;

    emit velocity_measured(scenes_[scene_current_].bodies_[pick_ix]->name.c_str(), -v_projected);
    reticule_.dot_active_ = true;
  }else{
    emit velocity_not_measured();
    reticule_.dot_active_ = false;
  }

}
