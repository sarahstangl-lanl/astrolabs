/**
 *
 * Author: Stou Sandalski <sandalski@astro.umn.edu>
 * License: Apache 2.0
 *
 * Description: Contains Qt UI construction and handling
 *
 */

#include "expansion_gui.h"

#include <QtWidgets/QApplication>

#include "ui_expansion.h"
#include "expansion_scene.h"


#ifndef __DATE__
#define __DATE__ "nk"
#endif

#ifndef __TIME__
#define __TIME__
#endif

ExpansionLab::ExpansionLab(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ExpansionLab) {
  ui->setupUi(this);

  // For the updating the time display
  connect(ui->sceneWidget, &ExpansionLabWidget::time_updated, this,
          &ExpansionLab::updateTime);

  // For changing the directions
  connect(ui->sceneWidget, &ExpansionLabWidget::selection_updated, this,
          &ExpansionLab::updateSelection);

  ui->epochSlider->setValue(0);
  ui->epochSlider->setMinimum(slider_scale_ * ui->sceneWidget->T_past);
  ui->epochSlider->setMaximum(slider_scale_ * ui->sceneWidget->T_future);

//    this->setWindowTitle(this->windowTitle() + __DATE__ " " __TIME__);
  ui->buildLabel->setText("Build date: " __DATE__ " " __TIME__);

  // Reset the label
  updateSelection(0);

  // We are animating at 60fps
//  startTimer(16);
}

ExpansionLab::~ExpansionLab() {
  ui->sceneWidget->cleanup();
  delete ui;
}

/**
 * Fires when the Run / Clear button is pressed
 */
void ExpansionLab::on_resetButton_clicked(){

//  std::cout<<"ExpansionLab::on_resetButton_clicked "<<std::endl;

  // Reset the scene
  ui->sceneWidget->reset();

  // Reset the label
  updateSelection(0);

  // Reset the slider position. Block signals otherwise sceneWidget will update twice.
  ui->epochSlider->blockSignals(true);
  ui->epochSlider->setValue(0);
  ui->epochSlider->blockSignals(false);
}

/**
 * Fires when the Exit button is pressed
 */
void ExpansionLab::on_exitButton_clicked(){

//  std::cout<<"ExpansionLab::on_exitButton_clicked "<<std::endl;

  // Just set the time to 0
  QApplication::quit();
}

void ExpansionLab::on_epochSlider_valueChanged(int value) {
//  std::cout<<"ExpansionLab::on_epochSlider_valueChanged "<<value<<std::endl;
  ui->sceneWidget->setEpoch(value / float(slider_scale_));
  update();
}


void ExpansionLab::updateTime(float t){
  ui->timeLabel->setText(QString::number(t, 'f', 2) + " Gyr");
}

void ExpansionLab::updateSelection(int selected_count){

  int selections_todo = neighbors_required_ - selected_count + 1;

  if(selected_count == 0){
    ui->directionsLabel->setText("Select a Home Galaxy");
  }else if(selections_todo > 1){
    ui->directionsLabel->setText("Select " + QString::number(selections_todo) + " galaxies at different distances");
  }else if(selections_todo == 1){
    ui->directionsLabel->setText("Select one more galaxy");
  }else{
    ui->directionsLabel->setText("");
  }


}


/**
 * TODO: External Settings Needed
 *
 *  - Number of galaxy images
 *  - Number galaxies to create
 *
 *
 */

// TODO: Can probably do some cowboy shit with templates
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  a.setAttribute(Qt::AA_EnableHighDpiScaling, true);

  QMainWindow window;

  ExpansionLab lab(&window);
  window.setCentralWidget(&lab);
  window.setWindowTitle("Expansion");
  window.setStyleSheet("background-color: black;");

//  window.showFullScreen();
  window.showMaximized();
//  w.show();

  return a.exec();
}
