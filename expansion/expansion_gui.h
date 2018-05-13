/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description:
 *
 */


#ifndef H_EXPANSION_GUI
#define H_EXPANSION_GUI

#include <QtCore/QTime>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>


namespace Ui {
  class ExpansionLab;
}



class ExpansionLab: public QWidget {
Q_OBJECT

public:
  explicit ExpansionLab(QWidget *parent = 0);

  ~ExpansionLab();

private:
  int slider_scale_ = 100;
  int neighbors_required_ = 3;

private slots:
  /**
   *  Time / Epoch slider
   */
  void on_epochSlider_valueChanged(int value);

  /**
   * Start Over button
   */
  void on_resetButton_clicked();

  void on_exitButton_clicked();

  /**
   * Animation timer
   */

  void timerEvent(QTimerEvent *){};

private:
  void updateTime(float t);
  void updateSelection(int selected_count3);

private:
//  QTime time_;
  Ui::ExpansionLab *ui;
};

#endif // H_EXPANSION_GUI
