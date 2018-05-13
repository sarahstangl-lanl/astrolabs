/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: Contains the startup and main code for
 * the Dark Matter velocity curve measurement lab
 *
 */

#include <QtWidgets/QApplication>

#include "dm_gui.h"

#include "ui_dark_matter_lab.h"

#include <iostream>


//#include <qwt/qwt_legend.h>
//#include <qwt/qwt_plot_curve.h>

#include <qwt_legend.h>
#include <qwt_plot_curve.h>
#include <qwt_abstract_scale_draw.h>
#include <qwt_plot_marker.h>
#include <qwt_plot_picker.h>
#include <qwt_picker_machine.h>

#include "absorption_lines.h"

using namespace std;

DarkMatterLab::DarkMatterLab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DarkMatterLab),
    plot_signal_(new absorption_lines()),
    plot_rest_(new absorption_lines()){
  ui->setupUi(this);


  ui->buildLabel->setText("Build date: " __DATE__ " " __TIME__);

  /**
   * Configure QWT plot
   */
  // Setup data
  float y_extents = 1.2f;
  this->plot_rest_->noise_ = false;
  this->plot_rest_->has_signal_ = true;

  this->plot_signal_->x_shift_ = 0.0f;
  this->plot_signal_->noise_ = true;
  this->plot_signal_->has_signal_ = false;

  // Visual Settings
  QColor C_reference(0, 114, 178, 128);
  QColor C_measured(213, 94, 0, 255);

  // Any thicker than this sucks.
  int pen_width = 2;

  // Setup plots
  QPalette p = ui->redshiftPlot->palette();
  p.setColor(QPalette::Window, Qt::white);
  ui->redshiftPlot->setPalette(p);

  // Reference signal
  QwtPlotCurve *reference_curve = new QwtPlotCurve("Reference");
  reference_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  reference_curve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
  QPen reference_pen(C_reference);
  reference_pen.setWidth(pen_width);
  reference_curve->setPen(reference_pen);
  reference_curve->attach(ui->redshiftPlot);
  reference_curve->setData(plot_rest_);

  // Source signal
  QwtPlotCurve *source_curve = new QwtPlotCurve("Source");
  source_curve->setRenderHint(QwtPlotItem::RenderAntialiased);
  source_curve->setLegendAttribute(QwtPlotCurve::LegendShowLine, true);
  QPen measured_pen(C_measured);
  measured_pen.setWidth(pen_width);
  source_curve->setPen(measured_pen);
  source_curve->attach(ui->redshiftPlot);
  source_curve->setData(plot_signal_);


  // Vertical lines
  QwtPlotMarker *reference_mark = new QwtPlotMarker;
  reference_mark->setLineStyle(QwtPlotMarker::VLine);
  reference_mark->setXValue(0.0f);
  reference_mark->attach(ui->redshiftPlot);

  plot_mark_signal_ = new QwtPlotMarker;
  plot_mark_signal_->setLineStyle(QwtPlotMarker::VLine);
  plot_mark_signal_->setXValue(0.0f);
  plot_mark_signal_->attach(ui->redshiftPlot);
  plot_mark_signal_->hide();

  // Initialize the X scale
  setPlotScale(0);

  // Setup axis
  ui->redshiftPlot->setAxisScale(ui->redshiftPlot->yLeft, -0.f, y_extents);
  ui->redshiftPlot->enableAxis(QwtPlot::yLeft, false);

  ui->redshiftPlot->insertLegend(new QwtLegend(), QwtPlot::TopLegend);

  // Create the point picker
  // Picker with click point machine to provide point selection

//  d_picker->setTrackerPen(QColor(Qt::white));

  QwtPlotPicker *picker = new QwtPlotPicker(ui->redshiftPlot->canvas());
  picker->setStateMachine(new QwtPickerClickPointMachine);
//  picker->setMousePattern(0, Qt::LeftButton, Qt::SHIFT);

  picker->setRubberBandPen(QColor(Qt::green));
  picker->setRubberBand(QwtPicker::VLineRubberBand);

  connect(picker, SIGNAL (selected(QPointF)), this,
          SLOT(on_plotPointSelected(QPointF)));

  /**
   * Connect Signals
   */
  // For the updating the time display
  connect(ui->sceneWidget, &DarkMatterScene::velocity_measured, this,
          &DarkMatterLab::on_velocityMeasurement);

  connect(ui->sceneWidget, &DarkMatterScene::velocity_not_measured, this,
          &DarkMatterLab::on_velocityNotMeasurement);

  // Start animation timer
  startTimer(16);
}

DarkMatterLab::~DarkMatterLab(){
  ui->sceneWidget->cleanup();
  delete ui;
}

/**
 *
 * There are three scales 0, 1, 2 and are defined in the dm_scene_graph.h file
 *
 * @param scale_index
 */
void DarkMatterLab::setPlotScale(int scale_index){

  float scale = ui->sceneWidget->getScaleByIndex(scale_index);

  // Adjust the label
  setVelocity(scale, v_current_);

  // Set the scale to
  this->plot_signal_->set_scale(scale);
  this->plot_rest_->set_scale(scale);
  ui->redshiftPlot->setAxisScale(ui->redshiftPlot->xBottom, -1.0 * scale, 1.0 * scale);
}

/**
 *
 * Set the velocity label and wavelength shift.
 *
 * @param velocity
 */
void DarkMatterLab::setVelocity(float velocity, float scale){

  if(abs(velocity) < scale){
    ui->velocityLabel->setText(QString::number(velocity, 'f', 2));

    // Compute doppler shift
    float beta = velocity / 3e5f;
    float delta_lambda = lambda_rest_ * beta;

    ui->deltaLambdaLabel->setText(QString::number(delta_lambda, 'f', 2));
  }else{
    ui->velocityLabel->setText(label_no_signal_);
    ui->deltaLambdaLabel->setText(label_no_signal_);
  }

}


/**
 * Exit
 */
void DarkMatterLab::on_exitButton_clicked(){
  QApplication::quit();
}

/**
 * Turn on the lensing simulation
 *
 * @param checked
 */
void DarkMatterLab::on_lensingSimulationButton_toggled(bool checked){

}

void DarkMatterLab::on_massSlider_valueChanged(int mass){

  ui->massValue->setText(QString::number(mass));
  ui->sceneWidget->setLensMass(mass);
}

/**
 * Switching plot range using the range buttons
 *
 *
 * @param button
 */
void DarkMatterLab::on_rangeButtonGroup_buttonClicked(QAbstractButton *button){
//  std::cout<<"on_rangeButtonGroup_buttonClicked"<<std::endl;

  if(button->objectName() == "rangeButtonLow"){
    setPlotScale(0);
  }else if(button->objectName() == "rangeButtonMedium"){
    setPlotScale(1);
  }else if(button->objectName() == "rangeButtonHigh"){
    setPlotScale(2);
  }else{
    std::cerr << "DarkMatterLab::on_rangeButtonGroup_buttonClicked: Invalid button "
              << button->objectName().toStdString() << std::endl;
    return;
  }
  ui->redshiftPlot->replot();
}


/**
 * Navigating scenes
 * @param button
 */
void DarkMatterLab::on_sceneButtonGroup_buttonClicked(QAbstractButton *button){
//  std::cout<<"on_sceneButtonGroup_buttonClicked"<<std::endl;

  if(button->objectName() == "solarSystemButton"){
    ui->stackedWidget->setCurrentIndex(0);
    ui->sceneWidget->showScene(0);
  }else if(button->objectName() == "spiralGalaxyButton"){
    ui->stackedWidget->setCurrentIndex(0);
    ui->sceneWidget->showScene(1);
    ui->sceneWidget->showLensing(false);

  }else if(button->objectName() == "galaxyClusterButton"){
    ui->stackedWidget->setCurrentIndex(0);
    ui->sceneWidget->showScene(2);
    ui->sceneWidget->showLensing(false);

  }else if(button->objectName() == "lensingSimulationButton"){
    ui->sceneWidget->showScene(2);

    // Switch panels to show mass or plot
    ui->stackedWidget->setCurrentIndex(1);

    // Enable or disable the lensing model
    ui->sceneWidget->showLensing(true);

  }else{
    std::cerr << "DarkMatterLab::on_sceneButtonGroup_buttonClicked: Invalid button "
              << button->objectName().toStdString() << std::endl;
    return;
  }


}

/**
 *
 * Fires when a velocity is measured
 *
 * @param name
 * @param velocity
 */
void DarkMatterLab::on_velocityMeasurement(const char *name, float velocity){


#if 0
  const char *label_v = "v =";
  const char *label_units = "km/s";
  const char *label_no_signal = "v = ##.#km/s";

  if(abs(velocity) < ui->sceneWidget->getScale()){
    QString area_str = label_v + QString::number(velocity, 'f', 2) + label_units;
    ui->velocityLabel->setText(area_str);
  }else{
    ui->velocityLabel->setText(label_no_signal);
  }

#endif

//  float x_shift = velocity;// / ui->sceneWidget->getScale();

  plot_signal_->has_signal_ = true;
  plot_signal_->set_x_shift(-velocity);

#if 0
  plot_mark_signal_->setXValue(velocity);
  plot_mark_signal_->show();
#endif

  ui->redshiftPlot->replot();
  update();
}


/**
 *
 * Fires when nothing is measured to reset things
 *
 */
void DarkMatterLab::on_velocityNotMeasurement(){

  ui->velocityLabel->setText(label_no_signal_);
//  plot_mark_signal_->setXValue(velocity);
  plot_mark_signal_->hide();

  plot_signal_->has_signal_ = false;
  plot_signal_->set_x_shift(0.0f);

  v_current_ = 0.0f;

  ui->redshiftPlot->replot();
  update();
}

void DarkMatterLab::timerEvent(QTimerEvent *){
  // Drive the animation in the SceneGraph
  ui->sceneWidget->updateAnimation();
}

/**
 * Fires when the user clicks anywhere in the plot
 *
 * @param pos
 */
void DarkMatterLab::on_plotPointSelected(const QPointF &pos){
#if 0
  std::cout<<"DarkMatterLab::on_plotSelected "<<pos.x()<<", "<<pos.y()<<std::endl;
#endif

  v_current_ = (float) pos.x();

  setVelocity(v_current_, ui->sceneWidget->getScale());

  plot_mark_signal_->setXValue(v_current_);
  plot_mark_signal_->show();
  ui->redshiftPlot->replot();
}


int main(int argc, char *argv[]){
  QApplication a(argc, argv);

  QMainWindow window;

  DarkMatterLab lab(&window);
  window.setCentralWidget(&lab);
  window.setWindowTitle("Dark Matter");

//  w.showFullScreen();
  window.showMaximized();

  int app_status = a.exec();
#if 0
  std::cout<<"App exit with status "<<app_status<<std::endl;
#endif
  return app_status;
}
