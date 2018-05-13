#ifndef DARK_MATTER_MAIN_H
#define DARK_MATTER_MAIN_H

#include <QtWidgets/QAbstractButton>
#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <qwt_plot_marker.h>

#include "absorption_lines.h"

namespace Ui {
  class DarkMatterLab;
}

class DarkMatterLab : public QWidget {
Q_OBJECT

public:
  explicit DarkMatterLab(QWidget *parent = 0);

  ~DarkMatterLab();

  /**
   *
   * Set the plot scale based on the index
   *
   * @param scale_index
   */
  void setPlotScale(int scale_index);

  void setVelocity(float velocity, float scale);

private slots:

  void on_exitButton_clicked();

  void on_lensingSimulationButton_toggled(bool checked);

  void on_massSlider_valueChanged(int value);

  void on_rangeButtonGroup_buttonClicked(QAbstractButton *button);

  void on_sceneButtonGroup_buttonClicked(QAbstractButton *button);

  void on_velocityMeasurement(const char *name, float velocity);

  void on_velocityNotMeasurement();

  void timerEvent(QTimerEvent *event);

  void on_plotPointSelected(const QPointF &pos);

private:
  Ui::DarkMatterLab *ui;

  // The reference
  absorption_lines *plot_signal_;
  absorption_lines *plot_rest_;

  QwtPlotMarker *plot_mark_signal_;

  // Settings
  const char *label_no_signal_ = "####";

  // This is the H line (Ca+)
  float lambda_rest_ = 396.847f;

  // Currently selected velocity
  float v_current_ = 0.0f;
};

#endif // DM_MAIN_H
