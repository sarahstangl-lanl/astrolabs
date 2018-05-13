/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: Main Qt GUI code
 *
 */


#ifndef KEPLER_MAIN_H
#define KEPLER_MAIN_H

#include <QtCore/QTime>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>


namespace Ui {
  class KeplerLab;
}

class KeplerLab : public QWidget {
Q_OBJECT

public:
  explicit KeplerLab(QWidget *parent = 0);

  ~KeplerLab();

private slots:

  /**
   *  Data changes
   */
  void on_angleSpinBox_valueChanged(double value);

  void on_velocitySpinBox_valueChanged(double value);

  void onVectorArrowChanged(float velocity, float angle);

  void onAngleMinusButtonClicked(bool);

  void onAnglePlusButtonClicked(bool);

  /**
   * Run simulation or clear simulation
   */
  void on_runClearButton_clicked();

  /**
   * Sweep management
   */

  void on_clearSweepsButton_clicked();

#if 1
  void on_sweepButton_toggled(bool checked);
#else
  void on_sweepButton_pressed();

  void on_sweepButton_released();
#endif
  /**
   * Tools management
   */

  void on_circleButton_toggled(bool checked);

  void on_rulerButton_toggled(bool checked);


  /**
   * Misc
   */

  void on_exitButton_clicked();

  void timerEvent(QTimerEvent *);

private:
  void synchronizeInterface();

private:
  //QTime time_;
  Ui::KeplerLab *ui;
};

#endif // KEPLER_MAIN_H
