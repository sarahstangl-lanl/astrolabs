/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: Widget for displaying a single
 * star image (B or V) and support classes
 *
 *
 */




#ifndef STARIMAGE_H
#define STARIMAGE_H

#include <QtGui/QPen>
#include <QtGui/QPixmap>
#include <QtWidgets/QWidget>

#include <iostream>
#include <vector>

struct delete_from_vector {
  template <class T>
  void operator()(T *ptr) const{
    delete ptr;
  }
};

class Star {

public:
  Star(int id, float x, float y, float mag, float color, bool selected = false)
      : _id(id), _color(color), _mag(mag), _selected(selected), _x(x), _y(y){
  }

//private:
  int _id;
  float _color;
  float _mag;
  bool _selected;

  // Relative coordinates on image
  float _x;
  float _y;
};

class StarData {

public:
  StarData(){
  }

  ~StarData(){
    std::for_each(_stars.begin(), _stars.end(), delete_from_vector());
  }

  void addStar(Star *star){
    this->_stars.push_back(star);
  }

  Star *getStar(int ix){
    return _stars.at(ix);
  }

  virtual int size() const{
    return (int) _stars.size();
  }

private:
  std::vector <Star *> _stars;
};


class StarImage : public QWidget {
Q_OBJECT

public:
  explicit StarImage(QWidget *parent = 0);

  ~StarImage();

  int hitTest(int x, int y);

  void setColorChannel(int channel);

  void setSingleMode(bool mode);

  void setSelected(int selected);

  void setStarData(StarData *starData);

signals:

  void curser_move(int, int);

  void star_selected(bool, int);

public
  slots:

  void onSelectionChanged(bool selected, int star_id);

protected:
  void drawCursor(QPainter &painter, double x, double y, bool selected);

  void drawSelected(QPainter &painter);

  void mouseMoveEvent(QMouseEvent *event);

  void mouseReleaseEvent(QMouseEvent *event);

  void paintEvent(QPaintEvent *event);

private:
  /**
   * Settings
   */

  // Colorization scale for B-V images
  float C_scale_ = 0.8f;

  // Radius for star indicator
  float star_size_ = 48.0f;

  // Size of the 'X' that appears on top of tapped stars
  int cross_size_ = 12;

  // How big is the "star has been measured" rectangle
  int rect_size_ = 16;

  // Thickness of the circles
  int star_pen_width_ = 2;

  // Thickness of the crosses
  int cursor_pen_width_ = 3;

  bool single_selection_mode_ = false;

  // Something is selected
  bool selected_ = false;

  int mouse_x_;
  int mouse_y_;
  int image_h_;
  int image_w_;

  // Index of currently selected star
  int selected_star_;

  // Background image
  QPixmap background_;

  // List of stars
  StarData *star_data_ = nullptr;

  QPen cursor_pen_;
  QPen secondary_cursor_pen_;
  QPen star_indicator_pen_;
  QPen star_measured_pen_;
};

#endif // STARIMAGE_H
