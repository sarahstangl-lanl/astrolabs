/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: This is the 'meat' of the program
 *
 */


#ifndef KEPLER_SCENE_GRAPH_H
#define KEPLER_SCENE_GRAPH_H

#include <iostream>
#include <math.h>
#include <memory>
#include <random>
#include <vector>

#include "scene_graph.h"

#include <QImage>
#include <QPainter>
#include <QTime>
#include <QVBoxLayout>
#include <QWidget>
#include <QtWidgets/QOpenGLWidget>

/**
 * Draw a bunch of galaxies
 *
 */
struct GalaxySet : public msg::Node {


  // TODO: Move these settings to main expansion class
  // Quad dimension
  float galaxy_size_ = 0.125f;

  // Universe scale
#if 0
  float global_scale_ = 1.0f;

  // The minium separation between galaxies is determined by this
  float global_scale_min_ = 3.1f;

  // The maximum separation is determined by this value
  float global_scale_max_ = 10.0f;
#elif 0
  float global_scale_ = 1.0f;
  float global_scale_min_ = 1.0f;
  float global_scale_max_ = 11.0f;
#else
  float global_scale_ = 1.0f;
  float global_scale_min_ = 3.0f;
  float global_scale_max_ = 21.0f;
#endif

  // Size of the Galaxy Image tiles (128^2) [pixels]
  const static int galaxy_image_w = 128;
  const static int galaxy_image_h = 128;

  //
  int random_seed_ = 4910;

  // Galaxy size will be between these
  float scale_rand_min_ = 0.4f;
  float scale_rand_max_ = 1.0f;

  // Location is perturbed by +/- this
  float rand_scale_location_ = 0.1f;

  // The total number of galaxies to generate. This is independent from the number
  // of galactic images in the atlas or the number of billboards.
  int max_count_ = 0;

  struct s_galaxy{

    // Position in our galaxy coordinate system ({x, y} = ()
    float x = 0.0f;
    float y = 0.0f;

    // Random scale for this Billboard
    float scale_x = 1.0f;
    float scale_y = 1.0f;
    float rotation = 0.0f;

    bool selected = false;

    std::string name;

    // Texture coordinate: (top-left, bottom-right)
    float tex_rect[4] = {0.0f, 1.0f, 1.0f, 0.0f};

  } *galaxy_info_;

  /**
   * Stores images for the galaxies
   */
  msg::ImageAtlas galaxies_atlas_;
  msg::BillboardSet galaxies_;

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

  GalaxySet(int max_galaxy_count, int max_image_count, int max_visible_galaxies,
            int galaxies_tex_unit, int distance_labels_tex_unit)
      : max_count_(max_galaxy_count),
        galaxies_atlas_(galaxies_tex_unit, galaxy_image_w, galaxy_image_h, max_image_count),
        galaxies_(max_visible_galaxies) {

    // Galaxy Images
    galaxies_.vert_shader = "./assets/shaders/galaxy_set.vert";
    galaxies_.frag_shader = "./assets/shaders/galaxy_set.frag";

    galaxies_.atlas_tex_unit_ = galaxies_tex_unit;
    galaxies_.global_opacity_ = 1.0f;

    // Galaxy Data
    galaxy_info_ = new s_galaxy[max_galaxy_count];

    // Make
    galaxies_.quad_height_ = galaxies_.quad_width_ = galaxy_size_;
  }

  /**
   *
   * GalaxySet generate billboard locations
   *
   */
  void generate_locations(){

    using namespace std;

    /**
     *
     * Generate the random locations of the galaxies
     *
     */

    int rows = (int)sqrt(max_count_);

    int cols = max_count_ / rows;

    int galaxy_ix = 0;

    // Initialize Random Number Generator
#if 0
    std::random_device rd;
    std::mt19937 generator(rd());
#else
    std::mt19937 generator(random_seed_);
#endif

    std::uniform_real_distribution<> rand_rotation(0.0f, float(2 * M_PI));
    std::uniform_real_distribution<> rand_xy_scale(scale_rand_min_, scale_rand_max_);
    std::uniform_real_distribution<> rand_location(-rand_scale_location_, rand_scale_location_);

    float dx = 1.0f / float(cols);
    float dy = 1.0f / float(rows);

    float y = -0.5f;

    for(int j = 0; j < rows; ++j){

      float x = -0.5f;

      for(int i = 0; i < cols; ++i){
        galaxy_info_[galaxy_ix].x = x + (float)rand_location(generator);
        galaxy_info_[galaxy_ix].y = y + (float)rand_location(generator);

        galaxy_info_[galaxy_ix].scale_x = (float) rand_xy_scale(generator);
        galaxy_info_[galaxy_ix].scale_y = (float) rand_xy_scale(generator);
        galaxy_info_[galaxy_ix].rotation = (float) rand_rotation(generator);

        ++galaxy_ix;

//        cout<<"Galaxy_ ix "<<galaxy_ix<<" vs "<<(max_count_ - 1)
//            <<" true? "<<
//        cout<<" x "<<x<<", y "<<y<<endl;

        x += dx;
      }
      y += dy;
    }

  }

  /**
   *
   * GalaxySet init_resources
   *
   * @return
   */
  virtual bool init_resources(){

    // Galaxies
//    std::cout<<"Initialize Galaxies"<<std::endl;
    if(!(galaxies_.init_resources() && galaxies_atlas_.init_resources())){
      return false;
    };

    return true;
  }

  /**
   * Free resources
   */
  virtual void cleanup(){
    galaxies_.cleanup();
    galaxies_atlas_.cleanup();
  }

  /**
   *
   *  Call this after updating the galaxy images atlas so that the texture coordinates
   *  are correctly assigned.
   *
   */
  void update_texture_coordinates(){

    std::mt19937 generator(random_seed_);

    std::uniform_int_distribution<> rand_int(0, galaxies_atlas_.count_);

    for(int i = 0; i < max_count_; ++i){

#if 0
      // We want randomness in the images also, otherwise can develop artifacts
      int map_ix = rand_int(generator);
#else
      int map_ix = i;
#endif
      // Generate the texture coordinates
      galaxies_atlas_.get_tile_coordinates(map_ix, galaxy_info_[i].tex_rect);
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
  void update_positions(float t_normal, float eye_x, float eye_y){
    using namespace std;

#if 0
    cout<<" GalaxySet::update_positions t_normal "<<t_normal
        <<", eye_x = "<<eye_x
        <<" eye_y = "<<eye_y<<endl;
#endif
    // The position of the galaxies
    global_scale_ = global_scale_min_ + t_normal * global_scale_max_;

//    float eye_scaled_x = global_scale_ * eye_x;
//    float eye_scaled_y = global_scale_ * eye_y;

    galaxies_.count_ = 0;

    for(int i = 0; (i < max_count_) && (galaxies_.count_ < galaxies_.capacity_); ++i){
      // TODO: Check if the galaxy is enabled if its viewable on the screen
#if 0
      // Compute the coordinates
      galaxies_.info_[galaxies_.count_].position[0] = global_scale_ * galaxy_info_[i].x - eye_scaled_x;
      galaxies_.info_[galaxies_.count_].position[1] = global_scale_ * galaxy_info_[i].y - eye_scaled_y;
#else
      float x = galaxy_info_[i].x - eye_x;
      float y = galaxy_info_[i].y - eye_y;

      // If galaxies are selected we NEVER wrap them around because it will cause artifacts.

      // TODO: Fix the selection

//      if(!galaxy_info_[galaxies_.count_].selected){
        while(x > 0.5){x -= 1.0;}
        while(y > 0.5){y -= 1.0;}

        while(x < -0.5){x += 1.0;}
        while(y < -0.5){y += 1.0;}
//      }

      galaxies_.info_[galaxies_.count_].position[0] = global_scale_ * x;
      galaxies_.info_[galaxies_.count_].position[1] = global_scale_ * y;
#endif
      galaxies_.info_[galaxies_.count_].position[2] = -1e-5 * i;

      galaxies_.info_[galaxies_.count_].scale[0] = galaxy_info_[i].scale_x;
      galaxies_.info_[galaxies_.count_].scale[1] = galaxy_info_[i].scale_y;
      galaxies_.info_[galaxies_.count_].rotation = galaxy_info_[i].rotation;

      // Copy the texture coordinates for this specific tile
      for (int k = 0; k < 4; ++k) {
        galaxies_.info_[galaxies_.count_].tex[k] = galaxy_info_[i].tex_rect[k];
      }
      ++galaxies_.count_;
    }

    // Upload the data
    galaxies_.upload_billboards();
  }

  /**
   *
   * Select the
   *
   * @param x world x
   * @param y world y
   * @return selection index
   */
#if 1
  int hitSelect(float x, float y) {
    return galaxies_.hitSelect(x, y);
  }
#else
  int hitSelect(float x, float y){

    using namespace std;

    float hit_radius_sq = galaxies_.quad_width_ * galaxies_.quad_width_
                       + galaxies_.quad_height_ * galaxies_.quad_height_;

    float r_sq_min = 2 * hit_radius_sq;
    int hit_ix = -1;

    for(int i = 0; i < galaxies_.count_; ++i){

      float dx = (galaxies_.info_[i].position[0] - x);
      float dy = (galaxies_.info_[i].position[1] - y);

//      galaxies_.info_[galaxies_.count_].position[0] = global_scale_ * galaxy_info_[i].x - eye_scaled_x;
//      galaxies_.info_[galaxies_.count_].position[1] = global_scale_ * galaxy_info_[i].y - eye_scaled_y;
//      galaxies_.info_[galaxies_.count_].position[2] = -1e-5 * i;
#if 0
      cout<<"hitSelect x = "<<x<<", "<<y<<" : hit_radius_sq "<<hit_radius_sq<<endl;
      std::cout<<"galaxy[i] i = "<<i
               <<" hit_radius_sq = "<<hit_radius_sq
               <<" x= "<<x
               <<" y= "<<y
               <<" galaxy_x= "<<galaxies_.info_[i].position[0]
               <<" galaxy_y= "<<galaxies_.info_[i].position[1]
               <<std::endl;
#endif
      float r_sq = dx*dx + dy*dy;

//      cout<<i<<" r_sq "<<r_sq<<endl;

      if(r_sq < hit_radius_sq){

#if 0
        std::cout<<"hitSelect:: element_ix= "<<i
                 <<" hit_radius_sq = "<<hit_radius_sq
                 <<" dx= "<<dx
                 <<" dy= "<<dy
                 <<std::endl;
#endif
        if(r_sq < r_sq_min){
          r_sq_min = r_sq;
          hit_ix = i;
        }
      }
    }

    return hit_ix;
  }

#endif
  void render(Camera<float>& camera_){

    // Render the galaxies
    glBindTexture(GL_TEXTURE_2D, galaxies_atlas_.tex_);
    galaxies_.render(camera_);
  }
};


/**
 * The Qt OpenGL widget that draws the scene
 *
 * TODO: Move this to expansion_gui.
 */
class ExpansionLabWidget : public QOpenGLWidget {
Q_OBJECT

public:
  ExpansionLabWidget(QWidget *parent);

  /**
   * Reset everything to zero
   */
  void reset(){

    // Reset selections
    selections_count_ = 0;

    for(int i = 0; i < galaxies_.max_count_; ++i){
      galaxies_.galaxy_info_[i].selected = false;
    }

    // Time
    T_current_ = 0;

    // Pan
    eye_x_ = 0.0f;
    eye_y_ = 0.0f;

    updateTime();
  }

  void initializeGL();

  void cleanup();

  void resizeGL(int w, int h);

  void paintGL();

  void updateTime();

  void updateDistanceLabel(int index, float distance);

  /**
   *
   * Update the positions of the galaxies
   *
   * @param epoch is on the range [-1, 1]
   *
   */
  void setEpoch(float epoch){
//  std::cout << "ExpansionLabWidget::setEpoch " << epoch <<endl;
    T_current_ = epoch;

    updateTime();
  }

  /**
   * Event Handler for mouse, touch and gestures
   *
   * @param event
   * @return
   */
  bool event(QEvent* event);

  /**
   *
   * Pan this many pixels on screen.
   *
   * @param dx
   * @param dy
   */
  void pan(int dx, int dy);

  // TODO: Split this part off
  // TODO: These two routines are almost identical
  void load_galaxies(QString base_path, int image_count);

  void load_backgrounds(QString base_path, int image_count);


public:

  /**
   *
   * Timeline settings
   *
   *  The entire timeline is between [T_past, T_future]
   *
   *  [T_freeze_f, T_future] the galaxies are visible and they move
   *  [T_freeze_i, T_free_f] galaxies fade into existence / background fades out
   *
   *  [T_big_bang, T_freeze_i] Background is interpolated
   *
   */


  // How far back is the timeline
  float T_past = -15.0f;

  // How many years ahead do we go on the timeline.
  float T_future = 2.0f;

  // Time of Big-Bang
  float T_big_bang = -13.8f;

  // Galaxies fade in
  float T_fade_i = -11.5f;
  float T_fade_f = -11.0f;

  float T_galaxy_move = -12.0f;

  // Time at which the selection boxes and lines are turned off to avoid clutter
  float T_reticule_off = -11.0f;

  /**
   * Visual Settings
   */

#if 0
  int max_galaxies_ = 256;
  int max_visible_ = 256;
  int max_galaxy_image_count_ = 25;
#else
  int max_galaxies_ = 400;
  int max_visible_ = 400;
  int max_galaxy_image_count_ = 25;
#endif

  const static int max_selections_ = 16;

  // Label size in pixels
  const static int label_image_w_ = 128;
  const static int label_image_h_ = 64;

  int label_font_size_ = 16;

  // Selection Colors
  float C_select_home[4] = {1.0f, 1.0f, 0.0f, 1.0f};
  float C_select_other[4] = {0.0f, 1.0f, 0.0f, 1.0f};

  float C_connectors[4] = {0.5f, 1.0f, 0.0f, 1.0f};

  int max_background_image_count_ = 0;

  QColor C_padding_ = QColor(80, 80, 80, 80);
//  QColor C_padding_ = Qt::transparent;
  QColor C_text_ = QColor(10, 140, 20, 255);
  QImage label_image_;

  /**
   * Internal Settings
   */

  // Map texture unit
  const static int tex_unit_background_ = 0;
  const static int tex_unit_galaxies_ = 1;
  const static int tex_unit_distances_ = 3;

  /**
   * State
   */

  // Current time in [Gyr]
  float T_current_ = 0.0f;

  // Position of the Eye in the normalized CS
  float eye_x_ = 0.0f;
  float eye_y_ = 0.0f;

  /**
   * View Parameters
   */
  float near_ = -10.0f;
  float far_ = 10.0f;
  float top_ = -1.0f;
  float left_ = -1.0f;

  float center_[3] = {0.0, 0.0, 0.0};
  float up_[3] = {0.0, 1.0, 0.0};
  float eye_[3] = {0.0, 0.0, 7.0};

  // The camera
  Camera<float> camera_;

  /**
   *
   *  Selection management
   *
   */
  struct selection_st{
    int galaxy_id;
  };

  selection_st selections_[max_selections_];

  // If 0 then nothing is selected. index 0 is 'home'
  int selections_count_ = 0;

  struct {

    // Mouse position
    int x_last;
    int y_last;

    int dx;
    int dy;

    bool down = false;
    bool dragged = false;

  } mouse_;

  /**
   * Scene Graph
   */
  // Galaxy billboards
  GalaxySet galaxies_;

  // Labels for the distances
  msg::ImageAtlas distance_labels_atlas_;
  msg::BillboardSet distance_labels_;

  // Lines that connect the galaxies
  msg::Geometry distance_connectors_;

  // Selection boxes
  msg::BillboardSet selection_boxes_;

  // Background
  msg::FullScreenImage background_;

//  QTime animation_timer_;

signals:
  void time_updated(float time);

  void selection_updated(int selected_count);
};


#endif // KEPLER_SCENE_GRAPH_H
