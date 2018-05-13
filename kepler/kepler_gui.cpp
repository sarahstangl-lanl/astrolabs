/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: Contains Qt UI construction and handling
 *
 */

#include "kepler_gui.h"

#include <QFont>
#include <QtWidgets/QApplication>

#include "ui_kepler.h"
#include "kepler_scene_graph.h"


#if 0
const int WIDGET_FONT_SIZE = 8;
#else
const int WIDGET_FONT_SIZE = 16;
#endif

#if 0
const float RULER_FONT_SCALE = 1.0;
const float SWEEP_FONT_SCALE = 1.0;
#else
const float RULER_FONT_SCALE = 2.0;
const float SWEEP_FONT_SCALE = 1.0;
#endif


#ifndef __DATE__
#define __DATE__ "nk"
#endif

#ifndef __TIME__
#define __TIME__
#endif

KeplerLab::KeplerLab(QWidget *parent)
    : QWidget(parent), ui(new Ui::KeplerLab){

  ui->setupUi(this);

  connect(ui->sceneWidget, &KeplerScene::new_orbit, this, &KeplerLab::synchronizeInterface);
  connect(ui->sceneWidget, &KeplerScene::arrow_changed, this, &KeplerLab::onVectorArrowChanged);

  // Angle fine adjustment arrows
  connect(ui->angleMinusButton, &QPushButton::clicked, this, &KeplerLab::onAngleMinusButtonClicked);
  connect(ui->anglePlusButton, &QPushButton::clicked, this, &KeplerLab::onAnglePlusButtonClicked);

  ui->angleSpinBox->setValue(ui->sceneWidget->getDefaultAngle());
  ui->velocitySpinBox->setValue(ui->sceneWidget->getDefaultVelocity());
  ui->velocitySpinBox->setMaximum(ui->sceneWidget->getVelocityMax());

  ui->buildLabel->setText("Build date: " __DATE__ " " __TIME__);
  ui->sceneWidget->setInterfaceScale(RULER_FONT_SCALE, SWEEP_FONT_SCALE);

  // Initialize the fonts
  QFont font;
  font.setPointSize(WIDGET_FONT_SIZE);

  // Main Buttons
  ui->runClearButton->setFont(font);
  ui->clearSweepsButton->setFont(font);
  ui->sweepButton->setFont(font);
  ui->exitButton->setFont(font);
  ui->circleButton->setFont(font);
  ui->rulerButton->setFont(font);

  // Fine-adjustment buttons
  ui->angleLabel->setFont(font);
  ui->angleSpinBox->setFont(font);
  ui->angleMinusButton->setFont(font);
  ui->anglePlusButton->setFont(font);
  ui->angleUnitsLabel->setFont(font);

  // Velocity
  ui->velocityLabel->setFont(font);
  ui->velocitySpinBox->setFont(font);
  ui->velocityMinusButton->setFont(font);
  ui->velocityPlusButton->setFont(font);
  ui->velocityUnitsLabel->setFont(font);

  // We are animating at 60fps
  startTimer(16);

}

KeplerLab::~KeplerLab(){
  ui->sceneWidget->cleanup();
  delete ui;
}

/**
 *  Data changes
 */
void KeplerLab::on_angleSpinBox_valueChanged(double angle_degrees){
  ui->sceneWidget->setLaunchAngle(angle_degrees);
}

/**
 * Fires when the velocity spin box is changed.
 *
 * @param velocity [km/s]
 */
void KeplerLab::on_velocitySpinBox_valueChanged(double value){
//  std::cout<<"on_velocitySpinBox_valueChanged "<<value<<std::endl;
  ui->sceneWidget->setLaunchVelocity(value);
}

/**
 *
 * Fires when the
 *
 * @param velocity [km/s]
 * @param angle [radians]
 */
void KeplerLab::onVectorArrowChanged(float velocity, float angle){
#if 0
  std::cout<<"on_vectorArrow_Changed "<<velocity<<" angle "<<angle<<std::endl;
#endif
  ui->velocitySpinBox->blockSignals(true);
  ui->velocitySpinBox->setValue(velocity);
  ui->velocitySpinBox->blockSignals(false);

  ui->angleSpinBox->blockSignals(true);

  float angle_deg = 180.0f * angle / M_PI;

  if(angle_deg < ui->angleSpinBox->minimum()){
    angle_deg += 360.0f;
  }else if(angle_deg > ui->angleSpinBox->maximum()){
    angle_deg -= 360.0f;
  }

  ui->angleSpinBox->setValue(angle_deg);
  ui->angleSpinBox->blockSignals(false);
}

/**
 * Fine Adjustment buttons
 *
 */
void KeplerLab::onAngleMinusButtonClicked(bool){

  float step = (float) ui->angleSpinBox->singleStep();

  float new_value = ui->angleSpinBox->value() - step;

  if(new_value < ui->angleSpinBox->minimum()){
    new_value += 360.0f;
  }

  ui->angleSpinBox->setValue(new_value);
}

void KeplerLab::onAnglePlusButtonClicked(bool){

  float step = (float) ui->angleSpinBox->singleStep();

  float new_value = ui->angleSpinBox->value() + step;

  if(new_value > ui->angleSpinBox->maximum()){
    new_value -= 360.0f;
  }

  ui->angleSpinBox->setValue(new_value);
}


/**
 * Fires when the Run / Clear button is pressed
 */
void KeplerLab::on_runClearButton_clicked(){

  if(!ui->sceneWidget->isValid()){
    // Run
    ui->sceneWidget->runSimulation();

    ui->runClearButton->setText("New Orbit");
    ui->angleSpinBox->setDisabled(true);
    ui->velocitySpinBox->setDisabled(true);

    ui->velocityMinusButton->setDisabled(true);
    ui->velocityPlusButton->setDisabled(true);
    ui->angleMinusButton->setDisabled(true);
    ui->anglePlusButton->setDisabled(true);

  }else{
    // Clear
    ui->sceneWidget->newOrbit();
    ui->runClearButton->setText("Launch");
    ui->sweepButton->setDisabled(true);
    ui->clearSweepsButton->setDisabled(true);

    ui->angleSpinBox->setDisabled(false);
    ui->velocitySpinBox->setDisabled(false);
    ui->velocityMinusButton->setDisabled(false);
    ui->velocityPlusButton->setDisabled(false);
    ui->angleMinusButton->setDisabled(false);
    ui->anglePlusButton->setDisabled(false);
  }

  synchronizeInterface();
}

/**
 *
 * Sweep actions (press / release / clear)
 *
 */
void KeplerLab::on_clearSweepsButton_clicked(){
//  std::cout<<"KeplerLab::on_clearSweepsButton_clicked"<<std::endl;

  ui->sceneWidget->clearSweeps();
  ui->clearSweepsButton->setDisabled(true);
  ui->sweepButton->setDisabled(false);
  ui->sweepButton->setChecked(false);
}

#if 1
void KeplerLab::on_sweepButton_toggled(bool checked){

  if(checked){
    if(ui->sceneWidget->sweepsAvailable()){
      ui->sceneWidget->startSweep();
    }

    //
    ui->sweepButton->setText("Stop Sweep");
  }
  else
  {
    ui->sceneWidget->stopSweep();
    ui->clearSweepsButton->setDisabled(false);

    if(!ui->sceneWidget->sweepsAvailable()) {
      ui->sweepButton->setDisabled(true);
    }

    ui->sweepButton->setText("Start Sweep");
  }
}

#else
// This does the hold button to sweep.
void KeplerLab::on_sweepButton_pressed(){
//  std::cout<<"KeplerLab::on_sweepButton_pressed"<<std::endl;

  if(ui->sceneWidget->sweepsAvailable()){
    ui->sceneWidget->startSweep();
  }
}

void KeplerLab::on_sweepButton_released(){
//  std::cout<<"KeplerLab::on_sweepButton_released"<<std::endl;

  ui->sceneWidget->stopSweep();
  ui->clearSweepsButton->setDisabled(false);

  if(!ui->sceneWidget->sweepsAvailable()){
    ui->sweepButton->setDisabled(true);
  }
}
#endif

/**
 * Tools management
 */

void KeplerLab::on_circleButton_toggled(bool checked){
  ui->sceneWidget->showCircle(checked);
}

void KeplerLab::on_rulerButton_toggled(bool checked){
  ui->sceneWidget->showRuler(checked);
}

/**
 * Fires when the Run / Clear button is pressed
 */
void KeplerLab::on_exitButton_clicked(){
  QApplication::quit();
}

void KeplerLab::timerEvent(QTimerEvent *){
//  std::cout<<"KeplerLab::timerEvent"<<std::endl;

  // Drive the animation in the SceneGraph
  ui->sceneWidget->updateAnimation();
}

/**
 *
 * Enable or Disable GUI components to reflect animation state. This can more
 * elegantly be done with mutliple control signals but this is the shortest
 * solution... I think.
 *
 */
void KeplerLab::synchronizeInterface(){
  using namespace std;

//  cout<<"KeplerLab::synchronizeInterface "<<endl;

  ui->circleButton->setChecked(ui->sceneWidget->isCircleEnabled());
  ui->rulerButton->setChecked(ui->sceneWidget->isRulerEnabled());

  if(ui->sceneWidget->isValid()){
    ui->runClearButton->setText("New Orbit");

    if(ui->sceneWidget->isCrashed()){
      ui->sweepButton->setDisabled(true);
    }else{
      ui->sweepButton->setDisabled(false);
    }

    ui->angleSpinBox->setDisabled(true);
    ui->velocitySpinBox->setDisabled(true);

    ui->velocityMinusButton->setDisabled(true);
    ui->velocityPlusButton->setDisabled(true);
    ui->angleMinusButton->setDisabled(true);
    ui->anglePlusButton->setDisabled(true);

  }else{
    ui->runClearButton->setText("Launch");
    ui->sweepButton->setDisabled(true);
    ui->clearSweepsButton->setDisabled(true);

    ui->angleSpinBox->setDisabled(false);
    ui->velocitySpinBox->setDisabled(false);
    ui->velocityMinusButton->setDisabled(false);
    ui->velocityPlusButton->setDisabled(false);
    ui->angleMinusButton->setDisabled(false);
    ui->anglePlusButton->setDisabled(false);
  }
}

#ifndef EXTERNAL_MAIN

// TODO: Can probably do some cowboy shit with templates
int main(int argc, char *argv[]){
  QApplication a(argc, argv);

//  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
//  QGuiApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

  QMainWindow window;
  window.setStyleSheet("background-color: black;");

  KeplerLab lab(&window);
  window.setCentralWidget(&lab);
  window.setWindowTitle("Orbits with Newton's Laws");

//  window.showFullScreen();
  window.showMaximized();
//  window.show();

  return a.exec();
}

#endif
