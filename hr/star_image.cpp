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


#include <cmath>
#include <iostream>
#include <QtGui/QColor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtWidgets/QToolTip>

#include "star_image.h"

using namespace std;

StarImage::StarImage(QWidget *parent) :
    QWidget(parent){
  setMouseTracking(true);

  setCursor(Qt::BlankCursor);
  background_.load(":/hr/m15gc");

  float aspectRatio = background_.width() / float(background_.height());
  this->setMinimumHeight(background_.height() / 2);
  this->setMinimumWidth(aspectRatio * (background_.height() / 2));
  this->setSizePolicy(QSizePolicy::Expanding,
                      QSizePolicy::Expanding);

  /**
   * Visual Settings
   */
  QColor indicator_color(0, 114, 178, 128);
  QColor measured_color(213, 94, 0, 255);
  QColor C_cursor(Qt::white);
  QColor C_cursor_secondary(Qt::white);

  // Star indicators pen
  star_indicator_pen_.setColor(indicator_color);
  star_indicator_pen_.setWidth(star_pen_width_);

  // Measured stars pen
  star_measured_pen_.setColor(measured_color);
  star_measured_pen_.setWidth(star_pen_width_);

  // Cursor pen
  cursor_pen_.setColor(C_cursor);
  cursor_pen_.setWidth(cursor_pen_width_);

  // Secondary image pen
  secondary_cursor_pen_.setColor(C_cursor_secondary);
  secondary_cursor_pen_.setWidth(3);
}

StarImage::~StarImage(){

}

void StarImage::drawCursor(QPainter &painter, double x_pos, double y_pos, bool selected){

  // Draw the non-measured stars
  if(selected){
    painter.setPen(secondary_cursor_pen_);
    painter.drawEllipse(x_pos * image_w_ - star_size_ / 2,
                        y_pos * image_h_ - star_size_ / 2,
                        star_size_, star_size_);
  }else{
    painter.setPen(cursor_pen_);
    painter.drawRect(x_pos * image_w_ - rect_size_ / 2,
                     y_pos * image_h_ - rect_size_ / 2,
                     rect_size_, rect_size_);
  }

  // Draw the selection reticule
  painter.drawLine(0,
                   y_pos * image_h_,
                   x_pos * image_w_ - rect_size_ / 2,
                   y_pos * image_h_);

  painter.drawLine(x_pos * image_w_ + rect_size_ / 2,
                   y_pos * image_h_,
                   image_w_,
                   y_pos * image_h_);

  painter.drawLine(x_pos * image_w_,
                   0,
                   x_pos * image_w_,
                   y_pos * image_h_ - rect_size_ / 2);

  painter.drawLine(x_pos * image_w_,
                   y_pos * image_h_ + rect_size_ / 2,
                   x_pos * image_w_,
                   image_h_);
}

/**
 *
 * @param x
 * @param y
 * @return
 */
int StarImage::hitTest(int x, int y){

  using namespace std;

  // We want to select the closest point
  float min_distance = 0.25f * star_size_ * star_size_;
  int selected = -1;

  // First check for non-selected items
  for(int i = 0; i < star_data_->size(); ++i){
    Star *s = star_data_->getStar(i);

    if(s->_selected){
      continue;
    }

    float dx = s->_x * image_w_ - x;
    float dy = s->_y * image_h_ - y;

    float dr = dx * dx + dy * dy;

    if(min_distance > dr){
      selected = i;
      min_distance = dr;
    }
  }

  if(selected >= 0){
    return selected;
  }

  // Check for previously selected items.
  min_distance = 0.25f * star_size_ * star_size_;
  for(int i = 0; i < star_data_->size(); ++i){
    Star *s = star_data_->getStar(i);

    float dx = s->_x * image_w_ - x;
    float dy = s->_y * image_h_ - y;

    float dr = dx * dx + dy * dy;

    if(min_distance > dr){
      selected = i;
      min_distance = dr;
    }
  }




  return selected;
}

void StarImage::mouseMoveEvent(QMouseEvent *event){

  mouse_x_ = event->x();
  mouse_y_ = event->y();

  update();
}

void StarImage::mouseReleaseEvent(QMouseEvent *event){

  // Are we under an item?
  int item = hitTest(event->x(), event->y());

  if((item < 0) || (item >= star_data_->size())){
    emit star_selected(false, item);
    return;
  }

  Star *star = star_data_->getStar(item);

  // If it's disabled enable it
//    star->_selected = ! star->_selected;
  star->_selected = true;

  emit star_selected(star->_selected, item);

  update();
}

/**
 * Draw stuff
 */
void StarImage::paintEvent(QPaintEvent *){
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  // Draw the background image
  float aspect_ratio = background_.width() / float(background_.height());

  image_w_ = this->width();
  image_h_ = this->height();

  if(image_w_ > image_h_){
    image_w_ = aspect_ratio * this->height();
  }else{
    image_h_ = this->width() / aspect_ratio;
  }
  // HACK:
  if(image_w_ > this->width()){
    image_w_ = this->width();
    image_h_ = this->width() / aspect_ratio;
  }

  painter.drawPixmap(0, 0, image_w_, image_h_, this->background_);

  if(star_data_ == 0){
    return;
  }

  if(single_selection_mode_){

    if(selected_ && (selected_star_ < star_data_->size())){
      Star *s = star_data_->getStar(selected_star_);
      drawCursor(painter, s->_x, s->_y, true);
    }
    return;
  }

  // Draw the all stars that haven't been measured yet
  painter.setPen(star_indicator_pen_);

  for(int i = 0; i < star_data_->size(); ++i){
    Star *s = star_data_->getStar(i);

    if(s->_selected){
      continue;
    }

    // Image coordinates
    int x = int(s->_x * image_w_ - 0.5f * star_size_);
    int y = int(s->_y * image_h_ - 0.5f * star_size_);

    painter.drawEllipse(x, y, star_size_, star_size_);
  }

  // Draw the measured stars
  painter.setPen(star_measured_pen_);

  for(int i = 0; i < star_data_->size(); ++i){
    Star *s = star_data_->getStar(i);

    if(!s->_selected){
      continue;
    }

    // Image coordinates
    int x = s->_x * image_w_ - star_size_ / 2;
    int y = s->_y * image_h_ - star_size_ / 2;

    painter.drawEllipse(x, y, star_size_, star_size_);

    painter.drawLine(s->_x * image_w_ - cross_size_, s->_y * image_h_ - cross_size_,
                     s->_x * image_w_ + cross_size_, s->_y * image_h_ + cross_size_);

    painter.drawLine(s->_x * image_w_ + cross_size_, s->_y * image_h_ - cross_size_,
                     s->_x * image_w_ - cross_size_, s->_y * image_h_ + cross_size_);

  }

  drawCursor(painter, float(this->mouse_x_) / image_w_, float(this->mouse_y_) / image_h_, false);
}

/**
 *
 * Which colors are we seeing.
 *
 *
 * channel mapping:
 *
 * 0 = full color image
 * 1 = red
 * 2 = green
 * 3 = blue
 * 4 = red + green
 * 5 = green + blue
 * 6 = red + blue
 * 7 = inverted
 */

void StarImage::setColorChannel(int channel){

  if(channel == 0){
    return;
  }

  QImage image = background_.toImage();

  for(int i = 0; i < background_.width(); ++i){
    for(int j = 0; j < background_.height(); ++j){
      QColor color(image.pixel(i, j));

      switch(channel){

        case 1:
          image.setPixel(i, j, qRgb(color.red(), 0, 0));
          break;
        case 2:
          image.setPixel(i, j, qRgb(0, color.green(), 0));
          break;
        case 3:
          image.setPixel(i, j, qRgb(0, 0, color.blue()));
          break;
        case 4:
          image.setPixel(i, j, qRgb(color.red(), color.green(), 0));
          break;
        case 5:
          image.setPixel(i, j, qRgb(color.blue(), color.blue(), color.blue()));
          break;
        case 6:
          image.setPixel(i, j, qRgb(color.red(), color.red(), color.red()));
          break;
        case 50:
          image.setPixel(i, j, qRgb(C_scale_ * color.blue(), C_scale_ * color.blue(), color.blue()));
          break;
        case 60:
          image.setPixel(i, j, qRgb(color.red(), C_scale_ * color.red(), C_scale_ * color.red()));
          break;
        case 7:
          image.setPixel(i, j, qRgb(color.black(), color.black(), color.black()));
          break;
      }
    }
  }

  background_ = QPixmap::fromImage(image);
}

void StarImage::setSelected(int selected){
  selected_star_ = selected;
}

void StarImage::setSingleMode(bool mode){
  this->single_selection_mode_ = mode;
}

void StarImage::setStarData(StarData *starData){
  this->star_data_ = starData;
}

void StarImage::onSelectionChanged(bool selected, int star_id){

  if(!single_selection_mode_){
    return;
  }

  this->selected_ = selected;
  this->selected_star_ = star_id;
  this->update();
}
