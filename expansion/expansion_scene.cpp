/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: Contains 3D rendering code for the
 * scene. The QOpenGLwidget code.
 *
 */

#include "expansion_scene.h"

#include <cmath>
#include <ctime>
#include <iostream>

#include <QGestureEvent>
#include <QImageReader>
#include <QVBoxLayout>
#include <QWidget>

#if USE_GLEW == 0
#include <QtGui/QOpenGLContext>
#endif

#include <QtWidgets/QGestureEvent>
#include <QtWidgets/QFileDialog>


using namespace std;

// Modified version of this https://github.com/openscenegraph/osg/blob/master/examples/osgviewerQt/CMakeLists.txt
ExpansionLabWidget::ExpansionLabWidget(QWidget *parent)
    : QOpenGLWidget(parent),
      max_background_image_count_(6),
      galaxies_(max_galaxies_, max_galaxy_image_count_, max_visible_,
                tex_unit_galaxies_, tex_unit_distances_),
      distance_labels_atlas_(tex_unit_distances_, label_image_w_, label_image_h_, max_selections_),
      distance_labels_(max_selections_),
      distance_connectors_(max_selections_),
      selection_boxes_(max_selections_),
      background_(tex_unit_background_, 1024, 1024, 6){

  setMouseTracking(true);
  setFocusPolicy(Qt::ClickFocus);

  // Enable Touch
  setAttribute(Qt::WA_AcceptTouchEvents, true);
  setAttribute(Qt::WA_TouchPadAcceptSingleTouchEvents, true);

//  grabGesture(Qt::TapGesture);
  grabGesture(Qt::PanGesture);
//  grabGesture(Qt::PinchGesture);

  // Setup OpenGL 3.3
  QSurfaceFormat glFormat;
  glFormat.setVersion(3, 3);
  glFormat.setProfile(QSurfaceFormat::CoreProfile);
  setFormat(glFormat);

  // Assign custom shader to lines
  distance_connectors_.vert_shader = "./assets/shaders/connector_lines.vert";
  distance_connectors_.frag_shader = "./assets/shaders/connector_lines.frag";

  /**
   * Initialize Data
   */

  for(int i = 0; i < max_selections_; ++i){
    selections_[i].galaxy_id = 0;
  }

  // Temp image for label generation
  label_image_ = QImage(label_image_w_, label_image_h_, QImage::Format_RGBA8888);
}

/**
 *
 * Free OpenGL and other resources
 *
 */
void ExpansionLabWidget::cleanup(){
  galaxies_.cleanup();
  background_.cleanup();
}

/**
 *
 * http://doc.qt.io/qt-5/qtopengl-hellogl2-glwidget-cpp.html
 *
 */
void ExpansionLabWidget::initializeGL(){

  cout << "ExpansionLabWidget::initializeGL" << endl;

#if USE_GLEW == 1
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

  /**
   * Initialize the galaxies (TODO: Move to settings)
   */
  std::string galaxies_base = "./assets/billboards/galaxies/GAL";
  load_galaxies(galaxies_base.c_str(), max_galaxy_image_count_);

  /**
   * Background Image
   */
  std::string expansion_base = "./assets/backgrounds/expansion/epoch_";
  load_backgrounds(expansion_base.c_str(), max_background_image_count_);


  /**
   * Galaxy Selections
   */
//  std::cout << "Initialize Selection Boxes" << std::endl;

  // Selection indicators
  selection_boxes_.vert_shader = "./assets/shaders/billboard_set.vert";
  selection_boxes_.frag_shader = "./assets/shaders/selection_box.frag";
  selection_boxes_.quad_width_ = 1.1f * galaxies_.galaxy_size_;
  selection_boxes_.quad_height_ = 1.1f * galaxies_.galaxy_size_;

  // The selection boxes all have normalized texture coordinate (TODO: Maybe this should be the default in BillboardSet)
  for(int i = 0; i < max_selections_; ++i){
    selection_boxes_.info_[i].tex[0] = 0.0;
    selection_boxes_.info_[i].tex[1] = 1.0;
    selection_boxes_.info_[i].tex[2] = 1.0;
    selection_boxes_.info_[i].tex[3] = 0.0;
  }

  selection_boxes_.init_resources();
  selections_count_ = 0;
//  ++selection_boxes_.count_;

  // DEBUG
#if 0
  //  selection_boxes_.init_grid(4);
  //  selection_boxes_.upload_billboards();

    selections_count_ = 16;
    for(int i = 0; i < selections_count_; ++i){
      selections_[i].galaxy_id = i;
  //    distance_labels_atlas_.add_tile();
    }
#endif

  /**
   * Distance Indicators
   */

//  std::cout << "Initialize Labels" << std::endl;

  distance_labels_.atlas_tex_unit_ = tex_unit_distances_;
  distance_labels_.global_opacity_ = 1.0f;

  // TODO: Move to settings
  distance_labels_.quad_width_ =  2.0f * galaxies_.galaxy_size_;
  distance_labels_.quad_height_ = 0.5f *  distance_labels_.quad_width_;

  distance_labels_.init_resources();

  // TODO: Why are we setting the count here?
  distance_labels_atlas_.count_ = max_selections_ - 1;
  distance_labels_atlas_.init_resources();

  for(int i = 0; i < max_selections_ - 1; ++i){
    // Generate the texture coordinates
    distance_labels_atlas_.get_tile_coordinates(i, distance_labels_.info_[i].tex);
  }
  distance_labels_.upload_billboards();

  // Distance lines
//  std::cout << "Initialize Distance Lines" << std::endl;
  distance_connectors_.init_resources();

  updateTime();
}

/**
 * Load the galaxy textures
 *
 * @param base_path
 * @param image_count
 */
void ExpansionLabWidget::load_galaxies(QString base_path, int image_count){

  // Initialize galaxy billboards
  galaxies_.init_resources();
  galaxies_.galaxies_atlas_.bind();

  // TODO: Move into galaxies_
  // Load the galaxy images

  for(int i = 0; i < image_count; ++i){

    QString filename = base_path + QString::number(i) + ".png";
    QImageReader reader(filename);
    const QImage img = reader.read();

    if(img.isNull()){
      cerr << "Cannot load: " << filename.toStdString() << ", " << reader.errorString().toStdString() << endl;
      continue;
    }
//    galaxy_img.save("galaxy_img.png");

    cout << "Loaded file " << filename.toStdString() << "  ( " << img.width() << " x " << img.height() << " )"
         << endl;

    QImage tile_image(galaxies_.galaxies_atlas_.tile_width_, galaxies_.galaxies_atlas_.tile_height_,
                      QImage::Format_RGBA8888);

    // Clear the image otherwise we have weird stuff happening
    tile_image.fill(qRgba(0, 0, 0, 0));

    QPainter painter(&tile_image);

    // DEBUG:
//    painter.fillRect(tile_image.rect(), Qt::red);

    // Draw the image
    painter.drawImage(QRect(1, 1, tile_image.width() - 2, tile_image.height() - 2), img);
    galaxies_.galaxies_atlas_.add_tile(tile_image.bits());

//    total_count++;
//    galaxies_.galaxies_atlas_.save_atlas("galaxies_atlas.png");
//    break;

  }
  galaxies_.galaxies_atlas_.save_atlas("galaxies_atlas.png");
  galaxies_.generate_locations();

  // Generate texture coordinates
  galaxies_.update_texture_coordinates();

}

/**
 *
 * Load the background texture for interpolating
 *
 * @param base
 * @param image_count
 */
void ExpansionLabWidget::load_backgrounds(QString base_path, int image_count){

  background_.init_resources();
  background_.bind();

  for(int i = 0; i < image_count; ++i){

    QString filename = base_path + QString::number(i) + ".png";
    QImageReader reader(filename);
    const QImage img = reader.read();

    if(img.isNull()){
      cerr << "Cannot load: " << filename.toStdString()
           << ", " << reader.errorString().toStdString() << endl;
      continue;
    }
//    galaxy_img.save("galaxy_img.png");
    QImage tile_image(background_.tex_width_, background_.tex_height_,
                     QImage::Format_RGBA8888);
    tile_image.fill(qRgba(0, 0, 0, 0));

    QPainter painter(&tile_image);

    // DEBUG:
//    painter.fillRect(tile_image.rect(), Qt::red);

    // Draw the image
    painter.drawImage(tile_image.rect(), img);

    background_.upload_slice(i, tile_image.bits());

    cout << " Loaded file " << i << " : " << filename.toStdString()
         << "  ( " << img.width() << " x " << img.height() << " )"
         << endl;
//    break;
  }
  glFlush();


  check_GL_error("ExpansionLabWidget::load_backgrounds() exit ");
}

/**
 * Called When the view is resized
 *
 * @param w
 * @param h
 */
void ExpansionLabWidget::resizeGL(int w, int h){

//  std::cout << "ExpansionLabWidget::resizeGL " << w << " x " << h << std::endl;

  float aspect = float(w) / float(h);

  float top = top_;
  float left = left_;

  if(aspect <= 1.0){
    top /= aspect;
  }else{
    left *= aspect;
  }

  camera_.init_orthographic(w, h, left, top, near_, far_);
  camera_.init_model_view();
  camera_.look_at(center_, eye_, up_);

  glViewport(0, 0, w, h);
}


/**
 *
 *  Update the world, move galaxies, interpolate background, etc.
 *
 *
 */
void ExpansionLabWidget::updateTime(){
  check_GL_error("ExpansionLabWidget::updatePositions entry");

  // Galaxies are moving between ([T_freeze_f, T_future])
  float t_galaxy = (T_current_ - T_galaxy_move) / (T_future - T_galaxy_move);
  t_galaxy = t_galaxy > 1.0f ? 1.0f : t_galaxy;
  t_galaxy = t_galaxy < 0.0f ? 0.0f : t_galaxy;

  // Galaxies fade out between T_freeze_i and T_freeze_f
  float alpha_galaxy = (T_current_ - T_fade_i) / (T_fade_f - T_fade_i);
  alpha_galaxy = alpha_galaxy > 1.0f ? 1.0f : alpha_galaxy;
  alpha_galaxy = alpha_galaxy < 0.0f ? 0.0f : alpha_galaxy;

  // Background interpolation happens ([T_big_bang, T_fade_i])
  float t_background = (T_current_ - T_big_bang) / (T_fade_i - T_big_bang);
  t_background = t_background > 1.0f ? 1.0f : t_background;
  t_background = t_background < 0.0f ? 0.0f : t_background;

  // Background scales from 1.0 to 2.0 between [T_big_bang, T_fade_f]
  float background_scale = (T_current_ - T_big_bang) / (T_fade_f - T_big_bang);

  // Switch to normalized time
  galaxies_.update_positions(t_galaxy, eye_x_, eye_y_);
  galaxies_.galaxies_.global_opacity_ = alpha_galaxy;

  // Set the background opacity based on parameters from
  background_.alpha_ = 1.0f - galaxies_.galaxies_.global_opacity_;
  background_.time_ = t_background;
  background_.scale_[0] = 1.0 + background_scale;
  background_.scale_[1] = 1.0 + background_scale;

  // Reset
  distance_connectors_.clear();

  distance_labels_.count_ = 0;

  check_GL_error("ExpansionLabWidget:: before loop");

  // Update the positions of the selected billboards
  for(int selection_ix = 0; selection_ix < selections_count_; ++selection_ix){
    int galaxy_ix = selections_[selection_ix].galaxy_id;

    // Update positions (using BillboardSet.positions in 3-space)
    for(int i = 0; i < 3; ++i){
      selection_boxes_.info_[selection_ix].position[i] =
          galaxies_.galaxies_.info_[galaxy_ix].position[i];
    }

    // Assign color
    for(int i = 0; i < 4; ++i){
      selection_boxes_.info_[selection_ix].color[i] = (selection_ix == 0) ? C_select_home[i] : C_select_other[i];
    }

    // Processing non-home galaxies
    if(selection_ix > 0){
      int home_ix = selections_[0].galaxy_id;

      /**
       * Compute distance between galaxies
       */
      float D_x = galaxies_.galaxies_.info_[galaxy_ix].position[0] - galaxies_.galaxies_.info_[home_ix].position[0];
      float D_y = galaxies_.galaxies_.info_[galaxy_ix].position[1] - galaxies_.galaxies_.info_[home_ix].position[1];

      float ds = sqrt(D_x * D_x + D_y * D_y);

      float unit_scale = 8.0f;
      float D = ds * unit_scale;

      // Update distance label
      updateDistanceLabel(selection_ix - 1, D);

      // and set the distance label position to be on top of (TODO: Z-layer)
      for(int i = 0; i < 3; ++i){
        distance_labels_.info_[selection_ix - 1].position[i] = 0.5f * (galaxies_.galaxies_.info_[home_ix].position[i]
                                                      + galaxies_.galaxies_.info_[galaxy_ix].position[i]);
      }
      ++distance_labels_.count_;

      // Add the line
      distance_connectors_.addPoint(galaxies_.galaxies_.info_[home_ix].position,
                                    C_connectors);
      distance_connectors_.addPoint(galaxies_.galaxies_.info_[galaxy_ix].position,
                                    C_connectors);
    }
  }

//  cout<<"Selections count "<<selections_count_<<endl;
  selection_boxes_.count_ = selections_count_;

  // Need to update the VBOs
  selection_boxes_.upload_billboards();
  distance_labels_.upload_billboards();

  distance_connectors_.upload();

  emit time_updated(T_current_);

  update();
}

/**
 * Create / update the distance label
 *
 * @param index
 * @param distance
 */
void ExpansionLabWidget::updateDistanceLabel(int index, float distance){

  check_GL_error("ExpansionLabWidget::updateDistanceLabel entry()");

  // Create a label
  distance_labels_atlas_.bind();
#if 0
  QImage image_(label_image_w_, label_image_h_, QImage::Format_RGBA8888);

  for(int x = 0; x < image_.width(); ++x){
    for(int y = 0; y < image_.height(); ++y){
      image_.setPixel(x, y, qRgba(0, 0, 0, 0));
    }
  }
#else
  label_image_.fill(0);

#endif

  QPainter painter(&label_image_);
  QFont font = painter.font();
  font.setPointSize(label_font_size_);
  font.setWeight(QFont::DemiBold);
  painter.setFont(font);

  QPen pen(C_text_);

  painter.setPen(pen);
  painter.fillRect(label_image_.rect(), C_padding_);
  painter.drawText(QRect(0, 0, label_image_w_, label_image_h_),
                   Qt::AlignCenter | Qt::AlignBaseline,
                   QString::number(distance, 'f', 2) + " Mpc");

  distance_labels_atlas_.update_tile(index, label_image_.bits());
//  distance_labels_atlas_.save_atlas("expansion_distance_labels.png");

  check_GL_error("ExpansionLabWidget::updateDistanceLabel exit()");
};

/**
 *
 * Draw the actual scene. This is done back to front.
 *
 */
void ExpansionLabWidget::paintGL(){

//  std::cout << "ExpansionLabWidget::paintGL" << std::endl;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  bool reticule_visible = (T_current_ > T_reticule_off);

  if(T_current_ > T_fade_i){
    // Draw galaxies
    galaxies_.render(camera_);

    if(reticule_visible){
      distance_connectors_.render(camera_);
    }

    if(reticule_visible){
      // Labels for any distance comparisons
      glBindTexture(GL_TEXTURE_2D, distance_labels_atlas_.tex_);
      distance_labels_.render(camera_);

      // The selection boxes
      selection_boxes_.render(camera_);
    }
  }

  // Draw the background
  if((T_current_ > T_big_bang) && (T_current_ < T_fade_f)){
    background_.render(camera_);
  }

  check_GL_error("ExpansionLabWidget::paintGL() exit");
}

/**
 * Handle mouse and gestures
 *
 * @param event
 * @return
 */
bool ExpansionLabWidget::event(QEvent *event){
//  std::cout<<"event "<<event->type()<<endl;

  float world_cs[4] = {0.0f, 0.0f, 0.0f, 0.0f};

  /*
   * Handle Gestures
   */
  if(event->type() == QEvent::Gesture){
//    const QGestureEvent* e = static_cast<QGestureEvent *>(event);
    std::cout << "Gesture event" << endl;
    return true;
//  } else if (event->type() == QEvent::NativeGesture){
//    const QGestureEvent* e = static_cast<NativeGesture *>(event);
//    std::cout<<"NativeGesture event"<<endl;
//    return true;
  }else if(event->type() == QEvent::TouchBegin){
    std::cout << "TouchBegin event" << endl;
//    const QTouchEvent* e = static_cast<QTouchEvent *>(event);
//    std::cout<<"TouchEvent at "<<e->x()<<", "<<e->y()<<endl;
//
//    float x = e->x();
//    float y = e->y();
//
//    camera_.unproject(x, y, -1.0f, world_cs);
//    std::cout << "hitTest Mouse = "<<x<<", "<<y<<std::endl;
//
//    int hit_index = galaxies_.hitSelect(world_cs[0], world_cs[1]);
//
//    if(hit_index > 0){
//      std::cout<<"Hit Something "<<std::endl;
//    }else {
//      mouse_.down_ = true;
//      mouse_.x_last_ = e->x();
//      mouse_.y_last_ = e->y();
//    }

    return true;
  }else if(event->type() == QEvent::MouseMove){
    const QMouseEvent *e = static_cast<QMouseEvent *>(event);

    if(mouse_.down){

      float dx = mouse_.x_last - e->x();
      float dy = e->y() - mouse_.y_last;
      mouse_.x_last = e->x();
      mouse_.y_last = e->y();

      mouse_.dx += dx;
      mouse_.dy += dy;
      pan(dx, dy);

      float dist_sq = dx * dx + dy * dy;

      if(dist_sq > 0){
        mouse_.dragged = true;
      }
//      std::cout << "MouseMove dx/dy " << mouse_.dx << ", " << mouse_.dy << endl;
    }

    return true;
  }else if(event->type() == QEvent::MouseButtonPress){
    const QMouseEvent *e = static_cast<QMouseEvent *>(event);
//    std::cout << "MouseButtonPress at " << e->x() << ", " << e->y() << endl;

    float x = e->x();
    float y = e->y();
    camera_.unproject(x, y, -1.0f, world_cs);

    // TODO: hit test on a tighter radius so that if the user taps right at the center
    // of the galaxy we know that they were trying to select it vs dragging.

////    std::cout << "hitTest Mouse = " << x << ", " << y << std::endl;
//
//    int hit_index = galaxies_.hitSelect(world_cs[0], world_cs[1]);
//
//    if(hit_index >= 0){
////      std::cout << "Hit Galaxy " << hit_index << std::endl;
//
//      // Check if the galaxy was previously selected
//      bool previously_selected  = false;
//
//      for(int i = 0; i < selections_count_; ++i){
//        if(selections_[i].galaxy_id == hit_index){
//          previously_selected = true;
//          break;
//        }
//      }
//
//
//      if((!previously_selected) && (selections_count_ + 1 < max_selections_)){
//        selections_[selections_count_].galaxy_id = hit_index;
//        ++selections_count_;
//
//        // Notify GUI that something has been selected
//        emit selection_updated(selections_count_);
//
//        updateTime();
//      }
//
//    }else{
    mouse_.down = true;
    mouse_.dragged = false;
    mouse_.dx = 0;
    mouse_.dy = 0;
    mouse_.x_last = x;
    mouse_.y_last = y;
//    }

    return true;
  }else if(event->type() == QEvent::MouseButtonRelease){

    const QMouseEvent *e = static_cast<QMouseEvent *>(event);
//    std::cout << "MouseButtonRelease at " << e->x() << ", " << e->y() << endl;

    int hit_index = -1;

    if(!mouse_.dragged){
      float x = e->x();
      float y = e->y();

      camera_.unproject(x, y, -1.0f, world_cs);
//    std::cout << "hitTest Mouse = " << x << ", " << y << std::endl;

      hit_index = galaxies_.hitSelect(world_cs[0], world_cs[1]);
    }

    if(hit_index >= 0){
//      std::cout << "Hit Galaxy " << hit_index << std::endl;

      // Check if the galaxy was previously selected
      bool previously_selected = false;

      for(int i = 0; i < selections_count_; ++i){
        if(selections_[i].galaxy_id == hit_index){
          previously_selected = true;
          break;
        }
      }

      if((!previously_selected) && (selections_count_ + 1 < max_selections_)){
        selections_[selections_count_].galaxy_id = hit_index;
        ++selections_count_;

        galaxies_.galaxy_info_[hit_index].selected = true;


        // Notify GUI that something has been selected
        emit selection_updated(selections_count_);
        updateTime();
      }
    }

    mouse_.down = false;
    return true;
  }

  return QWidget::event(event);
}

/**
 *
 * Pan dx,dy pixels on screen. This is dependent on the
 * scale and essentially rotates the eye vectors.
 *
 * This routine doesn't do anything special since we are on
 * a very large sphere and can only rotate a few degrees at
 * a time so nothing weird should happen
 *
 * @param dx
 * @param dy
 */
void ExpansionLabWidget::pan(int dx, int dy){

  using namespace std;

#if 0
  cout << "ExpansionLabWidget::pan dx=" << dx << ", dy = " << dy << endl;
#endif

  // Turn off panning when we start seeing the background
  if(T_current_ < T_fade_f ){
    return;
  }

  float eye_scale = 0.0001f;// * galaxies_.global_scale_;

  eye_x_ += eye_scale * dx;
  eye_y_ += eye_scale * dy;

  updateTime();
}

