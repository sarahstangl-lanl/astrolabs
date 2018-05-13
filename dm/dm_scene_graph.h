/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: 3D code including nodes for drawing
 * lab specific objects
 *
 */

#ifndef DM_SCENE_GRAPH_H
#define DM_SCENE_GRAPH_H

#include <iostream>
#include <random>
#include <string>
#include <vector>

// Include this before QtWidgets/QOpenGLWidget
#include "scene_graph.h"

#include <QTime>
#include <QWidget>
#include <QtWidgets/QOpenGLWidget>


#if 0

/**
 * Draw a bunch of galaxies
 *
 */
struct ClusterGalaxies : public msg::Node {


  // TODO: Move these settings to main expansion class
  // Quad dimension
  float galaxy_size_ = 0.125f;

  // Size of the Galaxy Image tiles (128^2) [pixels]
  const static int galaxy_image_w = 128;
  const static int galaxy_image_h = 128;

  /**
   * Stores images for the galaxies
   */
  msg::ImageAtlas atlas_;
  msg::BillboardSet billboards_;

  /**
   * GalaxySet contains information about N galaxies and can draw some subset of them on screen.
   *
   *
   * @param max_galaxy_count      Maximum number of galaxies we can operate on
   * @param max_image_count       The number of source galaxies
   * @param max_visible_galaxies  Maximum number of visible galaxies
   *
   * @param texture_unit
   */

  ClusterGalaxies(int galaxy_count, int galaxies_tex_unit = 8)
      : atlas_(galaxies_tex_unit, galaxy_image_w, galaxy_image_h, galaxy_count),
        billboards_(galaxy_count){

    // Galaxy Images
    billboards_.vert_shader = "./assets/shaders/galaxy_set.vert";
    billboards_.frag_shader = "./assets/shaders/cluster_galaxies.frag";
    billboards_.global_opacity_ = 1.0f;

    // Make
    billboards_.quad_height_ = billboards_.quad_width_ = galaxy_size_;
    billboards_.count_ = galaxy_count;

    update_texture_coordinates();
  }

  /**
   *
   * init_resources
   *
   * @return
   */
  virtual bool init_resources(){

    if(!(billboards_.init_resources() && atlas_.init_resources())){
      return false;
    };

    return true;
  }

  /**
   * Free Resources
   */
  void cleanup(){

    billboards_.cleanup();

    atlas_.cleanup();
  }

  /**
   *
   *  Call this after updating the galaxy images atlas so that the texture coordinates
   *  are correctly assigned.
   *
   */
  void update_texture_coordinates(){
    for(int i = 0; i < billboards_.count_; ++i){

#if 1
      billboards_.info_[i].tex[0] = 0.0;
      billboards_.info_[i].tex[1] = 1.0;
      billboards_.info_[i].tex[2] = 1.0;
      billboards_.info_[i].tex[3] = 0.0;
#else
      // Generate the texture coordinates
      atlas_.get_tile_coordinates(i, billboards_.info_[billboards_.count_].tex);
#endif
    }
  }


  /**
   *
   * @param galaxy_ix
   * @param x
   * @param y
   */
  void set_position(int galaxy_ix, const float position[]){

    if(galaxy_ix >= billboards_.count_){
      std::cerr << "ClusterGalaxies::set_position" << std::endl;
      return;
    }

    for(int i = 0; i < 3; ++i){
      billboards_.info_[galaxy_ix].position[i] = position[i];
    }
  }

  /**
   *
   * GalaxySet
   *
   * Update the galaxy positions based on the normalized time and eye location
   *
   *
   */
  void update_positions(){
    using namespace std;

    // Upload the data
    billboards_.upload_billboards();
  }

  void render(Camera <float> &camera_){
    // Render the galaxies
    glBindTexture(GL_TEXTURE_2D, atlas_.tex_);
    billboards_.render(camera_);
  }
};

#endif

/**
 *
 * Draw the galaxy, the lensing model and the legend
 *
 *
 * Need one texture atlas storing 3 tiles,
 * the legend, the galaxy image, lense target (lensed at specific time),
 * the lensed galaxy image
 *
 *
 */
struct ClusterLensing : public msg::Node {

  ClusterLensing(int lensing_texture_unit, int legend_texture_unit)
      : lensing_tex_unit_(lensing_texture_unit),
        legend_tex_unit_(legend_texture_unit),
        lensing_billboards_(2),
        lensing_atlas_(lensing_texture_unit, lensing_width_, lensing_height_, 2),
        legend_billboards_(1),
        legend_atlas_(legend_texture_unit, legend_width_, legend_height_, 1){

    lensing_billboards_.atlas_tex_unit_ = lensing_texture_unit;
    lensing_billboards_.quad_width_ = lensing_quad_width_;
    lensing_billboards_.quad_height_ = lensing_quad_height_;


    legend_billboards_.atlas_tex_unit_ = legend_texture_unit;
    legend_billboards_.quad_width_ = billboard_size_;
    legend_billboards_.quad_height_ = billboard_size_;
  }

  virtual bool init_resources(){

    std::cout << "ClusterLensing::init_resources" << std::endl;

    /**
     * Setup Geometry
     */

    // Lensing stuff
    lensing_billboards_.init_resources();
    lensing_atlas_.init_resources();

    for(int i = 0; i < 2; ++i){
      lensing_atlas_.add_tile();
      lensing_atlas_.get_tile_coordinates(i, lensing_billboards_.info_[i].tex);
    }

    // Legend
    legend_billboards_.init_resources();
    legend_atlas_.init_resources();
    legend_atlas_.add_tile();
    legend_atlas_.get_tile_coordinates(0, legend_billboards_.info_[0].tex);

    int tile_width = legend_atlas_.tile_width_;
    int tile_height = legend_atlas_.tile_width_;


    // initialize an image we'll distort later
    lensed_image_ = QImage(lensing_atlas_.tile_width_, lensing_atlas_.tile_height_, QImage::Format_RGBA8888);

    /**
     * Load the galaxies
     */
    // Bind the atlas
    lensing_atlas_.bind();

    // Read the galaxy image
    QImageReader reader(target_filename.c_str());
    const QImage img = reader.read();

    if(img.isNull()){
      std::cerr << "Error loading:  " << target_filename.c_str() << ", "
                << reader.errorString().toStdString() << std::endl;
      return false;
    }else{
      std::cout << "Loaded file " << target_filename.c_str()
                << "  ( " << img.width() << " x " << img.height() << " )"
                << std::endl;
    }

    QImage galaxy_image(img.width(), img.height(), QImage::Format_RGBA8888);
    galaxy_image.fill(qRgba(0, 0, 0, 0));
    QPainter galaxy_painter(&galaxy_image);
    galaxy_painter.drawImage(galaxy_image.rect(), img);

    /**
     * Draw the target image (galaxy + red countours on top of it)
     */
    target_image_ = QImage(img.width(), img.height(), QImage::Format_RGBA8888);
    target_image_.fill(qRgba(0, 0, 0, 0));
    QPainter target_painter(&target_image_);
    target_painter.setRenderHint(QPainter::Antialiasing, true);

    // Draw the target image large here
    target_painter.drawImage(galaxy_image.rect(), galaxy_image);

    // Draw the circles
    QPen contour_pen(C_dm_);
    contour_pen.setWidth(2);
    contour_pen.setColor(Qt::red);
    target_painter.setPen(contour_pen);

    for(int i = 1; i < 2; ++i){
      target_painter.drawEllipse(QRect(20 * i, 20 * i,
                                       img.width() - 40 * i,
                                       img.height() - 40 * i));
    }

    int half_w = (int) (0.5 * img.width());
    int half_h = (int) (0.5 * img.height());

    target_painter.drawLine(half_w - 4, half_h - 4, half_w + 4, half_h + 4);
    target_painter.drawLine(half_w + 4, half_h - 4, half_w - 4, half_h + 4);

//    target_painter.drawText(QRect(half_w, half_w, 10, 10),
//                            Qt::AlignCenter | Qt::AlignBaseline,
//                            "x");

//    target_painter.drawEllipse(QRect(half_w - 2, half_h - 2,
//                                     half_w + 2, half_h + 2));
    /**
     * Create the legend
     */
    QImage legend_image(legend_atlas_.tile_width_, legend_atlas_.tile_height_, QImage::Format_RGBA8888);
    legend_image.fill(qRgba(0, 0, 0, 0));

    QPainter legend_painter(&legend_image);

    QBrush brush(C_legend_);
    legend_painter.setBrush(brush);
    legend_painter.drawRect(QRect(0, 0, tile_width, 3 * tile_height / 4));

    // Galaxy Image
    legend_painter.drawImage(QRect(2, 2, tile_width / 4, tile_height / 4), galaxy_image);

    // Lensed image
    legend_painter.drawImage(QRect(2, 2 + tile_height / 4, tile_width / 4, tile_height / 4), target_image_);

    // Dark matter sprites
    QPen dm_pen(C_dm_);
    dm_pen.setWidth(4);
    //  dm_pen.setColor(Qt::red);
    legend_painter.setPen(dm_pen);

    int random_seed_ = 491;
    int point_count = 10;

    std::mt19937 generator(random_seed_);
    std::uniform_int_distribution <> rand_point(0, galaxy_image.width() / 8);

    for(int i = 0; i < point_count; ++i){
      int x = 20 + rand_point(generator);
      int y = (int) (20 + rand_point(generator) + 0.5 * tile_height);

      legend_painter.drawPoint(x, y);
    }

    // Text
    QFont font = legend_painter.font();
    font.setPointSize(legend_font_size_);
    font.setWeight(QFont::DemiBold);
    legend_painter.setFont(font);

    QPen pen(C_text_);

    legend_painter.setPen(pen);
    legend_painter.drawText(QRect(tile_width / 8, 0, tile_width, tile_height / 4),
                            Qt::AlignCenter | Qt::AlignBaseline,
                            "Lensed Galaxy");

    legend_painter.drawText(QRect(tile_width / 8, tile_height / 4, tile_width, tile_height / 4),
                            Qt::AlignCenter | Qt::AlignBaseline,
                            "Model Galaxy");

    legend_painter.drawText(QRect(tile_width / 8, tile_height / 2, tile_width, tile_height / 4),
                            Qt::AlignCenter | Qt::AlignBaseline,
                            "Dark Matter");

    // TODO: Why is this broken??
    //   lensing_atlas_.update_tile(1, galaxy_image.bits());

    // Load the target to atlas 2
    update_lens(0, M_true_, galaxy_image);

    // Load the dynamic image
    update_lens(1, M_, target_image_);

#if 0
    lensing_atlas_.save_atlas("lensing_atlas.png");
#endif

    lensing_billboards_.count_ = 2;

    // Target Lensed Image
    lensing_billboards_.info_[0].position[2] = 1.0f;

    // Lensed Image
    lensing_billboards_.info_[1].position[0] = 0.0f;
    lensing_billboards_.info_[1].position[2] = 1.5f;
    lensing_billboards_.upload_billboards();

    // Upload Legend
    legend_atlas_.bind();
    legend_atlas_.update_tile(0, legend_image.bits());
    legend_billboards_.count_ = 1;

    legend_billboards_.info_[0].position[0] = 1.8f;
    legend_billboards_.info_[0].position[1] = 0.0f;
    legend_billboards_.info_[0].position[2] = 1.2f;
    legend_billboards_.upload_billboards();

#if 0
    legend_atlas_.save_atlas("legend_atlas_.png");
#endif

    return check_GL_error("Reticule::initializeGL() exit");
  }

  void cleanup(){
    legend_billboards_.cleanup();
    lensing_billboards_.cleanup();

    lensing_atlas_.cleanup();
    legend_atlas_.cleanup();
  }

  /**
   *
   * Distort the image
   *
   * @param tile_ix
   * @param mass
   */
  void update_lens(int tile_ix, int mass, const QImage &source){

    float alpha = 1.0f;

    lensed_image_.fill(qRgba(0, 0, 0, 0));

    QPainter painter(&lensed_image_);

#if 1
    // http://leo.astronomy.cz/grlens/grl0.html
    int source_image_height = source.height();
    int source_image_width = source.width();

    int result_image_height = lensed_image_.height();
    int result_image_width = lensed_image_.width();

    int lens_x = result_image_width / 2;
    int lens_y = result_image_height / 2;

    int source_offset_x = galaxy_offset_h_ + (result_image_width - source_image_width) / 2;
//    int source_offset_x = (result_image_width - source_image_width)/2;
    int source_offset_y = (result_image_height - source_image_height) / 2;

    mass *= 0.9;

    for(int x = 0; x < result_image_width; ++x){
      for(int y = 0; y < result_image_height; ++y){

        float dx = x - lens_x;
        float dy = y - lens_y;

        float dsq = dx * dx + dy * dy;

        QRgb pixel = 0;

        if(dsq > 0){
          float tmp3 = source_offset_x + dx * mass * mass / dsq;
          float tmp4 = source_offset_y + dy * mass * mass / dsq;

          if((y - tmp4 >= 0) && (x - tmp3 >= 0)
             && (x - tmp3 < source_image_width)
             && (y - tmp4 < source_image_height)){

            pixel = source.pixel(x - tmp3, y - tmp4);

            pixel = qRgba(qRed(pixel),
                          qGreen(pixel),
                          qBlue(pixel),
                          alpha * qAlpha(pixel));

          }
        }
        lensed_image_.setPixel(x, y, pixel);
      }
    }

#else
    painter.drawImage(target_image_.rect(), target_image_);
#endif
//    lensed_image_.save("lensed_image_.png");

    lensing_atlas_.bind();
    lensing_atlas_.update_tile(tile_ix, lensed_image_.bits());

//    lensing_atlas_.save_atlas("lensing_atlas.png");
  }

  void setMass(int mass){
    M_ = mass;
    update_lens(1, M_, target_image_);
  }

  /**
   * Render the reticule and its label
   *
   * @param camera_
   */
  void render(Camera <float> &camera){

    // Lensing stuff
    lensing_atlas_.bind();
    lensing_billboards_.render(camera);

    // The legend
    legend_atlas_.bind();
    legend_billboards_.render(camera);

    check_GL_error("Lensing::render exit");
  }

  /**
   * Settings
   */
  float billboard_size_ = 1.0;

  float lensing_quad_width_ = 4.0f;
  float lensing_quad_height_ = 2.00f;

  // Label size in pixels / Texture Size
  const static int lensing_width_ = 1024;
  const static int lensing_height_ = 512;

  // Label size in pixels / Texture Size
  const static int legend_width_ = 256;
  const static int legend_height_ = 256;

  int legend_font_size_ = 18;

  // Legend background
  QColor C_legend_ = QColor(0, 0, 0, 255);

  // Legend text
  QColor C_text_ = QColor(64, 255, 64, 255);

  // The galaxy contours.
  QColor C_contours_ = QColor(255, 140, 20, 255);

  QColor C_dm_ = QColor((int) (0.1f * 255), (int) (0.1f * 255), (int) (0.8f * 255), 255);
  // {0.1f, 0.1f, 0.8f, 0.5f};

  // Image
  std::string target_filename = "./assets/backgrounds/ngc7742.png";

  /**
   * Internal
   */
  int lensing_tex_unit_ = 0;
  int legend_tex_unit_ = 0;

  // Labels for the distances
  msg::BillboardSet lensing_billboards_;
  msg::ImageAtlas lensing_atlas_;

  // Labels for the distances
  msg::BillboardSet legend_billboards_;
  msg::ImageAtlas legend_atlas_;

  // Image with galaxy + red countours on top of it
  QImage target_image_;

  // The distorted image we see
  QImage lensed_image_;

  int M_max_ = 500;
  int M_true_ = 246;
  int M_ = 0;

  int galaxy_offset_h_ = -150;
};

/**
 * Draw the velocity measurement reticule
 */
struct Reticule : public msg::Node {

  Reticule(int texture_unit)
      : tex_unit_(texture_unit),
        reticule_(1),
        reticule_label_(1){

    reticule_.setPosition(0, 0, 0);
    reticule_label_.setPosition(1.0, 0, 0);
    reticule_label_.atlas_tex_unit_ = texture_unit;

    label_image_ = QImage(label_image_w_, label_image_h_, QImage::Format_RGBA8888);

  }

  virtual bool init_resources(){

    std::cout << "ClusterLensing::init_resources" << std::endl;
    // Create the texture for the label
    glGenTextures(1, &tex_);

    glActiveTexture(GL_TEXTURE0 + tex_unit_);
    glBindTexture(GL_TEXTURE_2D, tex_);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);

    // These have to be after the load or it doesn't work.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, label_image_w_, label_image_h_,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    /**
     * Setup Programs
     */


    /**
     * Setup Geometry
     */

    reticule_.vert_shader = "./assets/shaders/billboard_set.vert";
    reticule_.frag_shader = reticule_frag_shader;
    reticule_.quad_width_ = 1.1f * reticule_size_;
    reticule_.quad_height_ = 1.1f * reticule_size_;

    reticule_.init_resources();
    dotColorHandle_ = glGetUniformLocation(reticule_.program_, "C_dot");

    // The reticule has normalized texture coordinates.
    reticule_.info_[0].tex[0] = 0.0;
    reticule_.info_[0].tex[1] = 1.0;
    reticule_.info_[0].tex[2] = 1.0;
    reticule_.info_[0].tex[3] = 0.0;
    reticule_.count_ = 1;


    reticule_label_.frag_shader = label_frag_shader;
    reticule_label_.init_resources();
    reticule_label_.quad_width_ = label_width_;
    reticule_label_.quad_height_ = label_height_;

    // The reticule has normalized texture coordinates.
    reticule_label_.info_[0].tex[0] = 0.0;
    reticule_label_.info_[0].tex[1] = 1.0;
    reticule_label_.info_[0].tex[2] = 1.0;
    reticule_label_.info_[0].tex[3] = 0.0;
    reticule_label_.count_ = 1;

    updateObjectLabel("Planetary Nebula");

    return check_GL_error("Reticule::initializeGL() exit");
  }

  void cleanup(){
    reticule_.cleanup();
    reticule_label_.cleanup();

    glDeleteTextures(1, &tex_);
  }

  /**
   * Generate the label
   *
   * @param object_name
   */
  void updateObjectLabel(const char *object_name){

    check_GL_error("Reticule::updateObjectLabel entry()");

    // Create a label
    glBindTexture(GL_TEXTURE_2D, tex_);
    label_image_.fill(0);

    if(object_name != nullptr){
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
                       object_name);
    }

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, label_image_w_, label_image_h_,
                    GL_RGBA, GL_UNSIGNED_BYTE, label_image_.bits());

    glGenerateMipmap(GL_TEXTURE_2D);

    // Need to have this here otherwise the caller can change buffer while we are editing it
    glFlush();

#if 0
    msg::ImageAtlas::save_atlas(tex_, label_image_w_, label_image_h_, "object_labels.png");
    label_image_.save("object_label_img.png");
#endif
    check_GL_error("ExpansionLabWidget::updateDistanceLabel exit()");
  };


  /**
   *  Show or hide the object label
   * @param show
   */
  void showObjectLabel(bool show){
    label_visible_ = show;
  }


  /**
   *
   *  Set the selector position
   *
   * @param x world coordinates
   * @param y
   */
  void updatePosition(float x, float y){

    // Need this for various reasons...
    setPosition(x, y, reticule_.info_[0].position[2]);

    reticule_.info_[0].position[0] = x;
    reticule_.info_[0].position[1] = y;
    reticule_.upload_billboards();

    reticule_label_.info_[0].position[0] = x;
    reticule_label_.info_[0].position[1] = y + label_offset_;
    reticule_label_.upload_billboards();
  }

  /**
   *
   * @param world_x
   * @param world_y
   * @return
   */
  virtual bool hitTest(float world_x, float world_y){
    using namespace std;
#if 0
    std::cout<<"Reticule::hitTest : x= "<<world_x
             <<", y= "<<world_y<<endl;
#endif
    return reticule_.hitTest(world_x, world_y);
  };


  /**
   *
   * @param x_i
   * @param y_i
   * @param x_f
   * @param y_f
   * @return
   */
  virtual bool handleDrag(float x_i, float y_i, float x_f, float y_f){

    float dx = x_f - x_i;
    float dy = y_f - y_i;

    float mouse_scale_ = 1.0f;

    float x_new = reticule_.info_[0].position[0] + mouse_scale_ * dx;
    float y_new = reticule_.info_[0].position[1] + mouse_scale_ * dy;

#if 0
    std::cout<<"Reticule::handleDrag "<<dx<<", "<<dy<<std::endl;
#endif
    updatePosition(x_new, y_new);

    return false;
  };


  /**
   * Render the reticule and its label
   *
   * @param camera_
   */
  void render(Camera <float> &camera){

    // Bind the texture map for the label
    glBindTexture(GL_TEXTURE_2D, tex_);

//    // TODO: This is a bit redundant...
//    glUseProgram(reticule_.program_);
//
//    if(true){
//      glUniform4fv(dotColorHandle_, 1, C_dot_active_);
//    }else{
//      glUniform4fv(dotColorHandle_, 1, C_dot_passive_);
//    }

    reticule_.render(camera);

    if(label_visible_){
      reticule_label_.render(camera);
    }

    check_GL_error("Reticule::render exit");
  }


  /**
   * Settings
   *
   *  Various things are contained inside the shaders also
   */

  const char *vert_shader = "./assets/shaders/billboard_set.vert";
  const char *reticule_frag_shader = "assets/shaders/reticule.frag";
  const char *label_frag_shader = "assets/shaders/billboard_set.frag";


  // Label size in pixels / Texture Size
  const static int label_image_w_ = 256;
  const static int label_image_h_ = 64;

  float reticule_size_ = 0.25f;
  float label_size_ = 0.8f * reticule_size_;

  float label_width_ = 4.0f * label_size_;
  float label_height_ = label_size_;
  float label_offset_ = reticule_size_;

  int label_font_size_ = 16;

  QColor C_padding_ = QColor(80, 80, 80, 80);
  QColor C_text_ = QColor(10, 140, 20, 255);

  float C_dot_active_[4] = {1.0f, 0.0f, 0.4f, 1.0f};
  float C_dot_passive_[4] = {0.0f, 0.4f, 1.0f, 1.0f};

  /**
   * Internal Objects
   */

  bool label_visible_ = false;
  bool dot_active_ = false;

  int tex_unit_ = 0;

  // Program handle
  GLuint reticule_program_;
  GLuint label_program_;

  msg::BillboardSet reticule_;
  msg::BillboardSet reticule_label_;

  // OpenGL
  GLuint tex_;

  GLuint reticulePositionHandle_;
  GLuint labelPositionHandle_;

  // Uniform handles
  GLint reticuleMvpHandle_;
  GLint labelMvpHandle_;
  GLint dotColorHandle_;

  GLint colorHandle_;


  // Qt
  QImage label_image_;

};

/**
 * A path constructed from the elements of an orbit.
 *
 */
struct OrbitalPath : msg::Path {


  // Semi-major axis
  float R;

  // Maximum velocity
  float V_max;

  // Orbital inclination
  float inclination;

  // Eccentricity
  float eccentricity;

  // Current element / tick, int(current_tick_) corresponds to current_element..
  float current_tick_ = 0;

  // Only allow one node per orbital path
  msg::Node *node_ = nullptr;

  OrbitalPath() : msg::Path(){

  }

  /**
   * Generate an orbit with
   *
   * @param a
   * @param v_0
   * @param inc
   * @param e
   * @param ccw     direction
   */
  void generate(float a, float v_0, float inc, float e){

    R = a;
    V_max = v_0;
    inclination = (float) (M_PI * inc / 180.0);
    eccentricity = e;

    using namespace std;

    float dtheta = 2.0f * float(M_PI) / float(capacity_ - 1);
    float theta = 0.0f;

    // We have a velocity in the other direction
    if(v_0 > 0.0f){
      theta = 2.0f * float(M_PI);
      dtheta = -dtheta;
    }

#if 0
    cout<<"Building orbit with "<<capacity_
        <<" R="<<a
        <<" V_max="<<V_max
        <<" inclination="<<inc
        <<" dtheta="<<dtheta<<endl;
#endif

#if 0
    float b = std::sqrt(1 - eccentricity * eccentricity);
#else
    float b = 1.0f;
#endif

    float sin_inc = sin(inclination);
    float cos_inc = cos(inclination);

    for(int i = 0; i < capacity_; ++i){
      float x = R * std::cos(theta);
      float y = 0.0f;
      float z = R * b * std::sin(theta);

      // Rotate by inclination
      float x_new = x * cos_inc + y * sin_inc;
      float y_new = -x * sin_inc + y * cos_inc;

      // Compute the velocity vector
      float v_x = V_max * std::sin(theta);
      float v_y = 0.0f;
      float v_z = V_max * b * std::cos(theta);

#if 0
      std::cout << std::setw(10) << v_x
                << std::setw(10) << v_y
                << std::setw(10) << v_z
                << std::endl;
#endif
      addPoint(x_new, y_new, z, v_x, v_y, v_z);
      ++draw_size;
      theta += dtheta;
    }
  }

  /**
   * Attach a node to the path
   *
   * @param node
   */
  void attach(msg::Node *node){
    node_ = node;
  }

  /**
   *
   * @param steps
   * @param position
   */
  void updatePosition(float steps, float position[]){

    if(node_ == nullptr){
      std::cerr << "OrbitalPath::updateAttachedPositions : Can't update positions of nullptr" << std::endl;
      return;
    }

    current_tick_ += steps;
    int index_i = int(current_tick_) % capacity_;
//    int index_f = int(current_tick_ + 1) % capacity_;

    for(int i = 0; i < 3; ++i){
      position[i] = data_[stride_ * index_i + i];
    }
  }

  /**
   *
   * Update the positions of attached nodes based on the given time.
   *
   * @param time
   */
  void updateAttachedPositions(float steps){

    float position[3];

    updatePosition(steps, position);

    node_->setPosition(position[0], position[1], position[2]);
  }

};

/*
 * Stores the parameters for one orbiting thing in the scene. For example information about Earth, Venus, etc.
 */
struct OrbitingObject {

  // Object name
  std::string name;

  float orbital_inclination;

  // in scaled coordinates?
  float orbital_radius;

  // in km/s in the real world
  float orbital_velocity;

  // Distance from camera
  float z_distance;

  // Distance from center of screen for stabilizing the sort (note used)
  float screen_distance;

  // Path of object
  OrbitalPath path;

  // Node for drawing object
  msg::Node *node;

//  int max_segments = 5096;
  int max_segments = 8096;

  // How many segments
  float segments_per_AU = 256.0f;

  OrbitingObject(msg::Node *node,
                 const std::string &name,
                 float radius,
                 float velocity,
                 float inclination,
                 float eccentricity)
      : name(name),
        orbital_inclination(inclination),
        orbital_radius(radius),
        orbital_velocity(velocity),
        path(),
        node(node){


    int segments = (int) (segments_per_AU * radius);
//    float segements_per_tick_ = 0.01f * segments / (float) (2.0f * M_PI * radius / fabs(velocity));

//    std::cout << "segments_per_tick_ " << segements_per_tick_ << std::endl;

    if(segments > max_segments){
      std::cerr << "OrbitingObject:: too many segments" << std::endl;
      throw 1;
    }

    path.init(segments);

    path.generate(radius, velocity, inclination, eccentricity);

    // TODO Attach where?
    if(node != nullptr){
      path.attach(node);
    }
  }
};


#if 1
/**
 * A spiral galaxy (for Scene 2)
 */
struct SpiralGalaxy : public msg::Billboard {

  /**
   * Settings
   */

  // Texture
  std::string texture_source_ = "./assets/textures/spiral_galaxy.jpg";

  // Shaders
  std::string vert_shader = "./assets/shaders/billboard.vert";
  std::string frag_shader = "./assets/shaders/billboard.frag";

  float scale_[2] = {1.0f, 1.0f};
  float radius_ = 2.0f;

  // Internal

  // Texture Size
  int tex_width_ = 0;
  int tex_height_ = 0;

  int capacity_ = 0;

  int count_ = 0;

  /**
   *  OpenGL
   */

  // Texture handles
  GLuint tex_;
  GLint tex_unit_ = 0;

  // Program handle
  GLuint program_;

  // Position Attribute
  GLuint positionHandle_;

  // Uniform handles
  GLint mvpHandle_;

  GLint scaleHandle_;
  GLint radiusHandle_;

  GLint colorSamplerHandle_;


  SpiralGalaxy(int tex_unit) : msg::Billboard(){
    tex_unit_ = tex_unit;

    // Geometry radius
    radius = 4.0f;
    setPosition(0.0f, 0.0f, 0.0f);

    scale_[0] = radius_ * 1.0f;
    scale_[1] = radius_ * 1.0f;
  }


  virtual bool init_resources(){
    check_GL_error("SpiralGalaxy::init_resources() entry");

    msg::Billboard::init_resources();

    /**
     * Build programs
     */
    program_ = glCreateProgram();

    if(!build_program(program_, "SpiralGalaxyProgram",
                      vert_shader.c_str(), frag_shader.c_str())){
      return false;
    }

    positionHandle_ = glGetAttribLocation(program_, "position_in_");

    mvpHandle_ = glGetUniformLocation(program_, "mvp_");
    radiusHandle_ = glGetUniformLocation(program_, "radius_");
    scaleHandle_ = glGetUniformLocation(program_, "scale_");

    colorSamplerHandle_ = glGetUniformLocation(program_, "color_sampler_");

    setup_array(positionHandle_);

    /**
     * Textures
     */

    // Load image
    QImageReader reader(texture_source_.c_str());
    const QImage source_img = reader.read();

    tex_width_ = source_img.width();
    tex_height_ = source_img.height();

    // Initialize texture objects
    glGenTextures(1, &tex_);

    glActiveTexture(GL_TEXTURE0 + tex_unit_);
    glBindTexture(GL_TEXTURE_2D, tex_);
//      glPixelStorei(GL_PACK_ALIGNMENT, 4);

    // These have to be after the load or it doesn't work.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Need an RGBA image so have to draw it ourselves.
    QImage img(tex_width_, tex_height_, QImage::Format_RGBA8888);
    QPainter painter(&img);
    painter.drawImage(source_img.rect(), source_img);

    // Upload
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width_, tex_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    // Need to have this here otherwise the caller can change buffer while we are editing it
    glFlush();

    return check_GL_error("SpiralGalaxy::init_resources() exit");
  }

  void cleanup(){
    glDeleteTextures(1, &tex_);
    glDeleteProgram(program_);
    msg::Billboard::cleanup();
  }

  virtual void render(Camera <float> &camera){

    glUseProgram(program_);
    glUniformMatrix4fv(mvpHandle_, 1, GL_FALSE, camera.mvp);

    glUniform1i(colorSamplerHandle_, tex_unit_);
    glUniform1f(radiusHandle_, radius);
    glUniform3fv(scaleHandle_, 1, scale_);

    msg::Billboard::render(camera);
  }


};
#else
struct SpiralGalaxy : public msg::FlatShape{

  /**
   * Settings
   */

  // Texture image
  std::string texture_source_ = "./assets/textures/spiral_galaxy.jpg";

  // Shaders
  std::string vert_shader = "./assets/shaders/identity.vert";
  std::string frag_shader = "./assets/shaders/spiral_galaxy.frag";

  // Galaxy size
  float radius_ = 0.75f;


  /**
   * Data
   */
  // Program handle
  GLuint program_ = 0;

  // Position Attribute
  GLint positionHandle_ = 0;
  GLint mvpHandle_ = 0;

  GLint backgroundHandle_ = 0;
  GLint backgroundAlpha_ = 0;
  GLint backgroundScale_ = 0;
  GLint inclinationHandle_ = 0;

  // Background texture
  GLuint tex_ = 0;


  /**
   * Texture
   */
  int tex_unit_ = 0;

  int tex_width_ = 0;
  int tex_height_ = 0;
  int layers_ = 0;

  float alpha_ = 1.0f;
  float inclination_ = 0.0f;

  // Scale of quad
  float scale_[3] = {1.0f, 1.0f, 1.0f};

  SpiralGalaxy(int texture_unit)
      : tex_unit_(texture_unit){

  }

  /**
   *
   * @return
   */
  virtual bool init_resources(){
    check_GL_error("SpiralGalaxy::init_resources() entry");
    std::cout<<"SpiralGalaxy::init_resources"<<std::endl;

    /**
     * Build programs
     */
    program_ = glCreateProgram();

    bool status = build_program(program_, "SpiralGalaxy",
                                vert_shader.c_str(), frag_shader.c_str());
    if(!status){
      return false;
    }

    positionHandle_ = glGetAttribLocation(program_, "position_in_");

    mvpHandle_ = glGetUniformLocation(program_, "mvp_");
    backgroundHandle_ = glGetUniformLocation(program_, "background_");
    backgroundAlpha_ = glGetUniformLocation(program_, "background_alpha_");
    backgroundScale_ = glGetUniformLocation(program_, "scale_");
    inclinationHandle_ = glGetUniformLocation(program_, "inclination_");

    /**
     * Initialize textures
     */
    // Load image
    QImageReader reader(texture_source_.c_str());
    const QImage source_img = reader.read();

    tex_width_ = source_img.width();
    tex_height_ = source_img.height();

    // Initialize texture objects
    glGenTextures(1, &tex_);

    glActiveTexture(GL_TEXTURE0 + tex_unit_);
    glBindTexture(GL_TEXTURE_2D, tex_);
//      glPixelStorei(GL_PACK_ALIGNMENT, 4);

    // These have to be after the load or it doesn't work.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Need an RGBA image so have to draw it ourselves.
    QImage img(tex_width_, tex_height_, QImage::Format_RGBA8888);
    QPainter painter(&img);
    painter.drawImage(source_img.rect(), source_img);

    // Upload
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_width_, tex_height_, 0, GL_RGBA, GL_UNSIGNED_BYTE, img.bits());
    glGenerateMipmap(GL_TEXTURE_2D);

    /**
     * Create geometry
     */
    msg::FlatShape::init_resources();
    msg::FlatShape::setup_array(positionHandle_);
    msg::FlatShape::build_triangle(2.0);

    return check_GL_error("FullScreenImage::init_resources() exit");
  }

  /**
   * FullScreenImage::cleanup
   *
   *  Free OpenGL resources
   *
   */
  void cleanup(){
    glDeleteProgram(program_);
    glDeleteTextures(1, &tex_);
  }

  void setScaleFromAspect(float aspect){
    scale_[0] = radius_ * 1.0f;
    scale_[1] = radius_ * aspect;
  }

  /**
   * FullScreenImage::render
   *
   * @param camera
   */
  virtual void render(Camera<float>& camera){
//    check_GL_error("SpiralGalaxy::init_resources() entry");

    glUseProgram(program_);

    glUniform1i(backgroundHandle_, tex_unit_);

    glUniform1f(backgroundAlpha_, alpha_);
    glUniform3fv(backgroundScale_, 1, scale_);

    if(inclinationHandle_ >= 0) glUniform1f(inclinationHandle_, inclination_);

    msg::FlatShape::bind();
    msg::FlatShape::render(camera);

    check_GL_error("SpiralGalaxy::render() exit");
  }
};

#endif

/**
 *
 * A bipolar nebula like eta-car
 *
 */
struct BipolarNebula : public msg::ProceduralSphere {

  std::string name_ = "Bipolar Nebula";


  BipolarNebula(){
    frag_shader = "./assets/shaders/bipolar_nebula.frag";
    radius = 0.4f;
  }
};

struct DarkCloud : public msg::ProceduralSphere {

  std::string name_ = "Dark Cloud";

  DarkCloud(){
    frag_shader = "./assets/shaders/cloud.frag";
    radius = 0.2f;

    // Set the color to dark gray
    color_[0] = 0.6f;
    color_[1] = 0.6f;
    color_[2] = 0.6f;
  };

};

/**
 *
 * A cluster of stars
 *
 * // TODO: Star variation
 * // TODO: Add glare
 *
 */
struct StarCluster : public msg::Sprites {

  std::string name_ = "Star Cluster";
  float R_equatorial_ = 0.1f;

  float color_[4] = {0.8f, 0.9f, 1.0f, 1.0f};

  int random_seed_ = 4910;

  // Radii distribution
  float star_radii_[2] = {1.0f, 8.0f};

  msg::SpriteProgram stars_program_;


  StarCluster(int star_count)
      : msg::Sprites(star_count){
  };


  bool init_resources(){

    using namespace std;

    std::cout << "StarCluster::init_resources" << std::endl;

    // Initialize the program
    stars_program_.init_resources();
    stars_program_.build();

    msg::Sprites::init_resources();
    msg::Sprites::setup_array(stars_program_.positionHandle_,
                              stars_program_.sizeHandle_,
                              stars_program_.colorHandle_);


    std::mt19937 generator(random_seed_);
    std::uniform_real_distribution <> angular_rand(-1, 1);
    std::uniform_real_distribution <> r_rand(0.01 * R_equatorial_, R_equatorial_);
    std::uniform_real_distribution <> r_star_rand(star_radii_[0], star_radii_[1]);


    for(int i = 0; i < capacity_; ++i){

      float sin_theta = (float) angular_rand(generator);
      float cos_theta = sqrt(1 - sin_theta * sin_theta);

      float sin_phi = (float) angular_rand(generator);
      float cos_phi = sqrt(1 - sin_phi * sin_phi);

      float r = (float) r_rand(generator);

      float x = r * cos_theta * sin_phi;
      float y = r * sin_theta * sin_phi;
      float z = r * cos_phi;

      float radius = (float) r_star_rand(generator);

      addPoint(x, y, z, radius, color_[0], color_[1], color_[2], color_[3]);
      ++draw_size;
    }

    upload();

    return true;
  }

  virtual void render(Camera <float> &camera){

#if 0
    std::cout<<"Star cluster render"<<std::endl;
    check_GL_error("StarCluster::render() enter");
#endif

    // Dark Matter sprites
    glUseProgram(stars_program_.program_);
    glUniformMatrix4fv(stars_program_.mvpHandle_, 1, GL_FALSE, camera.mvp);
    glUniform3fv(stars_program_.offsetHandle_, 1, position);

    msg::Sprites::render(camera);

    check_GL_error("StarCluster::render() exit");
  }


};


/**
 *
 * This is an H2 region
 *
 */
struct HIIRegion : public StarCluster {

  // The stars inside the HII region
  msg::ProceduralSphere envelope_;

  std::string name_ = "H II region";

  float R_equatorial_ = 0.5f;
  float R_star_ = 1.0f;

  HIIRegion(int star_count) : StarCluster(star_count){
    envelope_.frag_shader = "./assets/shaders/cloud.frag";
    envelope_.radius = 0.2f;

    // Set the color to red
    envelope_.color_[0] = 1.0f;
    envelope_.color_[1] = 0.0f;
    envelope_.color_[2] = 0.0f;

    // Set the random seed for the star cluster
    random_seed_ = 10331;
  }

  bool init_resources(){

    using namespace std;

    std::cout << "StarCluster::init_resources" << std::endl;

    StarCluster::init_resources();

    envelope_.init_resources();


    return true;
  }

  void cleanup(){
    envelope_.cleanup();

    StarCluster::cleanup();
  }

  /**
   * Update the position of the envelope
   */
  virtual void updateChildren(){
    envelope_.setPosition(position[0], position[1], position[2]);
  }

  virtual void render(Camera <float> &camera_){
    check_GL_error("HIIRegion::render() enter");


    StarCluster::render(camera_);

    // Now draw the gas layer outside
    envelope_.render(camera_);

    check_GL_error("HIIRegion::render() exit");
  }
};


/**
 * Draw a planetary nebula, it's orbital path, and animate orbit
 */
struct PlanetaryNebula : public msg::ProceduralSphere {

  std::string name_ = "Planetary Nebula";

//    float wd_radius = 0.005f;
//    float shell_radius = radius;

  PlanetaryNebula(){
    frag_shader = "./assets/shaders/cloud.frag";
    radius = 0.2f;

    // Set the color to dark gray
    color_[0] = 0.6f;
    color_[1] = 1.0f;
    color_[2] = 0.6f;
  }

};


/**
 * A cluster's galaxy (for Scene 3)
 */
struct ClusterGalaxy : public msg::ProceduralSphere {

  ClusterGalaxy(){
    radius = 0.125f;

    frag_shader = "./assets/shaders/cluster_galaxy.frag";

    // Set the color to dark gray
    color_[0] = 1.0f;
    color_[1] = 0.8f;
    color_[2] = 0.5f;
    color_[3] = 0.9f;
  }

};


/**
 *
 * Functional for the sorting
 *
 * @tparam T
 */
template <class T>
struct screen_distance_comparator {
  bool operator()(const OrbitingObject *lhs, const OrbitingObject *rhs){
    if (lhs->z_distance == rhs->z_distance){
      // Not sure why...
      return (lhs->orbital_radius < rhs->orbital_radius);
      //return (lhs.screen_distance < rhs.screen_distance);
    }else{
      return (lhs->z_distance > rhs->z_distance);
    }
  }
};

/*
 * Stores information about a single scene like the solar system or the galaxy clusters
 */
struct Scene {

  Scene(){

  }

  Scene(const std::string &name)
      : inclination_final(0),
        inclination_initial(0),
        max_equatorial_radius(0),
        max_orbital_radius(0),
        max_orbital_velocity(0),
        name(name){

  }

  void add_body(OrbitingObject *body){
    bodies_.push_back(body);
  }

  void setOrbitInclination(float initial, float final){
    inclination_final = final;
    inclination_initial = initial;
    inclination_delta = inclination_final - inclination_initial;
  }

  void resetInclination(){
    inclination = inclination_initial;
    T_current_ = 0.0f;
  }

  void setFinalEyeLocation(float x, float y, float z){
    eye_f_[0] = x;
    eye_f_[1] = y;
    eye_f_[2] = z;
  }

  /**
   *
   * Set the inclination
   *
   * t = [ms]
   * @return True if the inclination was updated...
   */
  bool updateInclination(float t_ms){


    // Update time
    T_current_ = T_current_ + t_ms;

    if(T_current_ > T_final_){
      T_current_ = T_final_;
      return false;
    }

    float t = T_current_ / T_final_;

    inclination = inclination_delta * t + inclination_initial;

#if 0
    std::cout<<" t= "<<t<<", "<<" inclination "<<inclination<<std::endl;
#endif

    // At zero inclination we are at our final UP and EYE positions
    float theta = -inclination;

    // In this case Z->X, and Y->Y
    // Rotate the up
    up_[2] = (float) (up_f_[2] * cos(theta) + up_f_[1] * sin(theta));
    up_[1] = (float) (-up_f_[2] * sin(theta) + up_f_[1] * cos(theta));

    // Rotate the eye
    eye_[2] = (float) (eye_f_[2] * cos(theta) + eye_f_[1] * sin(theta));
    eye_[1] = (float) (-eye_f_[2] * sin(theta) + eye_f_[1] * cos(theta));

    return true;
  }

  /**
   *
   * Sort the objects by view order
   *
   */
  void sort(){

    float world_position[4];

    // Update the z-position of each body
    for(size_t i = 0; i < bodies_.size(); ++i){
      vec4_by_mat4x4(camera_.mvp, bodies_[i]->node->position, world_position);
      bodies_[i]->z_distance = world_position[2];// / world_position[3];
//      body->screen_distance = pow(center_eye[0] / center_eye[3], 2.0) + pow(center_eye[1]/ center_eye[3], 2.0);
    }

    // Sort by the z position
    screen_distance_comparator <float> cmp;
    std::sort(bodies_.begin(), bodies_.end(), cmp);
  }

  /**
   * Internal data
   */
  std::string _background_texture;
  std::vector <OrbitingObject *> bodies_;

  // For the initial animation
  float inclination_final;
  float inclination_initial;
  float inclination_delta;
  float inclination;

  float T_current_ = 0.0f;

  // How many ticks it should take to transition
  // the inclination
  float T_final_ = 100.0f;

  float max_equatorial_radius;
  float max_orbital_radius;
  float max_orbital_velocity;

  std::string name;

  // The camera eye position for this scene
  float eye_[3];
  float up_[3];

  float eye_f_[3];
  float up_f_[3] = {0.0f, 1.0f, 0.0f};

  Camera <float> camera_;
};

/**
 *
 * The main graphics widget responsible for
 * rendering the scene
 *
 *
 */
class DarkMatterScene : public QOpenGLWidget {
Q_OBJECT

public:
  DarkMatterScene(QWidget *parent);

  /**
   * Called by system when the OpenGL sub-system is initialized.
   */
  void initializeGL();

  /**
   * Call this to free resources allocated in the initializeGL call
   */
  void cleanup();

  /**
   * Called by system when Window is drawn for first time or resized.
   *
   * Setup viewport and camera here
   *
   * @param w
   * @param h
   */
  void resizeGL(int w, int h);

  /**
   * Draw the scene
   */
  void paintGL();

  /**
   * Called every animation tick
   */
  void updateAnimation();

  /**
   * GUI Event funel
   *
   * @param event
   * @return
   */
  bool event(QEvent *event);

  /**
   * Call to take measurement under the reticule.
   */
  void takeMeasurement();

  /**
   * Transition to a specific scene
   *
   * where `scene_index` is
   *
   *   0 - Solar System
   *   1 - Spiral Galaxy
   *   2 - Galaxy Cluster
   *
   */
  void showScene(int scene_index){

    if((scene_index >= scene_count_) && (scene_index < 0)){
      std::cerr << "DarkMatterScene::showScene: invalid scene index"
                << scene_index << std::endl;
      return;
    }else if(scene_index == scene_current_){
      return;
    }

    scenes_[scene_index].resetInclination();

    // TODO: For the transition animation. Here set a target_scene

    scene_current_ = scene_index;

    // Update the reticule
    takeMeasurement();

    update();
  }

  /**
   * Turn on the lensing model
   * @param enabled
   */
  void showLensing(bool enabled){
    lensing_enabled_ = enabled;
    setLensMass(0);
  }

  /**
   * Set the mass of the dark matter lense
   * @param value
   */
  void setLensMass(float mass){
    M_dm_ = mass;
    dm_sprites_.draw_size = (int) (N_dm_sprites_ * (M_dm_ / M_dm_max_));

    // Set the mass of the lens
    lensing_.setMass(mass);

    update();
  }

  /**
   *
   * @param range_index 0 for 50, 1 for 300, 2 for 2500
   */
  float getScaleByIndex(int range_index){
    scale_ix_ = range_index;
    return x_scales_[scale_ix_];
  }

  /**
   * Return the maxium velocity visible on the plot
   *
   * @return
   */
  float getScale(){
    return x_scales_[scale_ix_];
  }

signals:

  /**
   * Fires when something is measured
   */
  void velocity_measured(const char *name, float velocity);

  /**
   * Fires when nothign is measured but a measurement was attempted
   */
  void velocity_not_measured();

protected:

  /**
   *
   * Settings
   *
   */

  // Velocity scales for the three scenes
  float x_scales_[3] = {50.0f, 300.0f, 2500.0f};

  // Maximum sprites
  const static int N_dm_sprites_ = 5000;

  // Maximum dark matter mass.
  float M_dm_max_ = 500.0f;

  // Current DM size
  float M_dm_ = 100.0f;

  // Color and Radius of sprites
  float C_dm_sprites_[4] = {0.1f, 0.1f, 0.8f, 0.5f};
  float R_dm_sprites_ = 5.0;


  // Map texture units
  const static int tex_unit_reticule_label_ = 7;

  const static int tex_unit_sun_ = 0;
  const static int tex_unit_mercury_ = 1;
  const static int tex_unit_venus_ = 2;
  const static int tex_unit_earth_ = 3;
  const static int tex_unit_mars_ = 4;

  const static int tex_unit_galaxy_ = 5;
  const static int tex_unit_cluster_ = 6;
  const static int tex_unit_lensing_ = 8;
  const static int tex_unit_legend_ = 9;


  /**
   *  Animation
   */

  QTime animation_time_;

  bool animation_ = false;
  float tick_per_ms_ = 1.0f / 48.0f;
  int T_prev_;

  /**
   * State
   */

  // Scale index
  int scale_ix_ = 0;

  /**
   * View Parameters
   */

  // Perspective
  float fov_ = 30.0f;
  float near_ = 0.10f;
  float far_ = 100.0f;

  // Orthographic for screen-space stuff like HUD and full screen image
  float ortho_near_ = -10.0f;
  float ortho_far_ = 10.0f;
  float top_ = -1.0f;
  float left_ = -1.0f;

  float center_[3] = {0.0f, 0.0f, 0.0f};
//  float up_[3] = {0.0f, 1.0f, 0.0f};

//  // We have 3 scenes with three separate eyes
//  float eye_[3] = {0.0f, 0.1f, 3.0f};
  float screen_up_[3] = {0.0f, 1.0f, 0.0f};
  float screen_eye_[3] = {0.0f, 0.1f, 3.0f};

  // The camera (moved to
//  Camera<float> camera_;

  // Camera for drawing HUD elements
  Camera <float> screen_camera_;

  // Mouse
  struct {

    // Mouse position
    int x_last;
    int y_last;

    int dx;
    int dy;

    // Button is down
    bool down = false;

    // We are dragging thing
    bool dragged = false;

    // The reticule is selected
    bool selected = false;

  } mouse_;

  /**
   * Scene Graph
   */

  // Measurement box (screen space)
  Reticule reticule_;

  // Program for drawing all paths
  msg::PathProgram pathProgram_;

  // Solar System
  msg::TexturedSphere sun_;
  msg::TexturedSphere mercury_;
  msg::TexturedSphere venus_;
  msg::TexturedSphere earth_;
  msg::TexturedSphere mars_;

  SpiralGalaxy spiral_galaxy_;

  // Spiral Galaxy
  BipolarNebula bipolar_nebula_;
  DarkCloud dark_cloud_;
  HIIRegion h2_region_;
  PlanetaryNebula planetary_nebula_;
  StarCluster star_cluster_;

  // Galaxy Cluster
  msg::FullScreenImage cluster_background_;

  const static int galaxies_ct_ = 5;
  ClusterGalaxy cluster_galaxy_[galaxies_ct_];
  ClusterLensing lensing_;
  msg::Sprites dm_sprites_;
  msg::SpriteProgram dm_sprites_program_;

  // Scenes
  int scene_current_ = 0;
  const static int scene_count_ = 3;

  bool lensing_enabled_ = false;

  Scene scenes_[scene_count_];

};

#endif // DM_SCENE_GRAPH_H
