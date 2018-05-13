#include <QtWidgets/QApplication>

#include "hr_main.h"

#include "ui_hr_diagram.h"

HrDiagramLab::HrDiagramLab(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::HrDiagramLab) {
  ui->setupUi(this);


  // Update build label
  ui->buildLabel->setText("Build date: " __DATE__ " " __TIME__);

  // Hardcoded function with all of the star data
  addStars();

  // Main image
  ui->starImage->setStarData(&_star_data);
  ui->starImage->setColorChannel(0);

  // The small images B and V
  ui->bImage->setStarData(&_star_data);
  ui->bImage->setSingleMode(true);
//  ui->bImage->setColorChannel(6);
  ui->bImage->setColorChannel(60);


  ui->vImage->setStarData(&_star_data);
  ui->vImage->setSingleMode(true);
//  ui->vImage->setColorChannel(5);
  ui->vImage->setColorChannel(50);


  // Connect signals
  connect(ui->starImage, SIGNAL(star_selected(bool, int)), this, SLOT(onSelectionChanged(bool, int)));
  connect(ui->starImage, SIGNAL(star_selected(bool, int)), ui->bImage, SLOT(onSelectionChanged(bool, int)));
  connect(ui->starImage, SIGNAL(star_selected(bool, int)), ui->vImage, SLOT(onSelectionChanged(bool, int)));
}

HrDiagramLab::~HrDiagramLab() {
  delete ui;
}

void HrDiagramLab::addStars() {

  _star_data.addStar(new Star(1, 0.0278481f, 0.591139f, 20.11f, 0.5f));
  _star_data.addStar(new Star(2, 0.0304557f, 0.261266f, 15.35f, 0.79f));
  _star_data.addStar(new Star(3, 0.0329114f, 0.560759f, 20.1f, 0.62f));
  _star_data.addStar(new Star(4, 0.0359747f, 0.753316f, 18.0f, 0.65f));
  _star_data.addStar(new Star(5, 0.0469114f, 0.658127f, 13.0f, 1.4f));
  _star_data.addStar(new Star(6, 0.0556962f, 0.201266f, 20.3f, 0.6f));
  _star_data.addStar(new Star(7, 0.0570633f, 0.437595f, 14.52f, 0.85f));
  _star_data.addStar(new Star(8, 0.0778228f, 0.907367f, 14.78f, 1.06f));
  _star_data.addStar(new Star(9, 0.0834937f, 0.670835f, 15.65f, 0.76f));
  _star_data.addStar(new Star(10, 0.0873418f, 0.18481f, 21.31f, 0.73f));
  _star_data.addStar(new Star(11, 0.0928861f, 0.307494f, 17.73f, 0.64f));
  _star_data.addStar(new Star(12, 0.106f, 0.334658f, 18.26f, 0.69f));
  _star_data.addStar(new Star(13, 0.106329f, 0.218987f, 18.51f, 0.57f));
  _star_data.addStar(new Star(14, 0.111392f, 0.583544f, 20.75f, 0.58f));
  _star_data.addStar(new Star(15, 0.111392f, 0.897468f, 18.32f, 0.66f));
  _star_data.addStar(new Star(16, 0.112658f, 0.868354f, 22.08f, 0.78f));
  _star_data.addStar(new Star(17, 0.116658f, 0.406025f, 18.51f, 0.61f));
  _star_data.addStar(new Star(18, 0.118987f, 0.773418f, 19.69f, 0.55f));
  _star_data.addStar(new Star(19, 0.118987f, 0.725316f, 18.93f, 0.55f));
  _star_data.addStar(new Star(20, 0.124051f, 0.849367f, 19.12f, 0.53f));
  _star_data.addStar(new Star(21, 0.134177f, 0.540506f, 20.75f, 0.56f));
  _star_data.addStar(new Star(22, 0.140203f, 0.481722f, 18.32f, 0.65f));
  _star_data.addStar(new Star(23, 0.141772f, 0.753165f, 16.8f, 0.77f));
  _star_data.addStar(new Star(24, 0.141772f, 0.950633f, 19.2f, 0.57f));
  _star_data.addStar(new Star(25, 0.146709f, 0.864937f, 16.48f, 0.75f));
  _star_data.addStar(new Star(26, 0.152962f, 0.609443f, 15.95f, 0.64f));
  _star_data.addStar(new Star(27, 0.154051f, 0.191013f, 19.11f, 0.5f));
  _star_data.addStar(new Star(28, 0.16157f, 0.901139f, 15.78f, 0.89f));
  _star_data.addStar(new Star(29, 0.16362f, 0.0309114f, 16.22f, 0.73f));
  _star_data.addStar(new Star(30, 0.172152f, 0.51519f, 18.35f, 0.66f));
  _star_data.addStar(new Star(31, 0.214506f, 0.0837215f, 14.93f, 0.79f));
  _star_data.addStar(new Star(32, 0.21519f, 0.879747f, 19.8f, 0.53f));
  _star_data.addStar(new Star(33, 0.223367f, 0.687443f, 16.44f, 0.78f));
  _star_data.addStar(new Star(34, 0.232228f, 0.121063f, 15.73f, 0.92f));
  _star_data.addStar(new Star(35, 0.235468f, 0.783646f, 13.25f, 1.36f));
  _star_data.addStar(new Star(36, 0.240506f, 0.289873f, 21.02f, 0.63f));
  _star_data.addStar(new Star(37, 0.242962f, 0.353215f, 16.22f, 0.84f));
  _star_data.addStar(new Star(38, 0.244304f, 0.0697722f, 15.12f, 0.96f));
  _star_data.addStar(new Star(39, 0.248101f, 0.388608f, 17.27f, 0.72f));
  _star_data.addStar(new Star(40, 0.254608f, 0.625797f, 15.95f, 0.56f));
  _star_data.addStar(new Star(41, 0.259291f, 0.749722f, 14.51f, 1.09f));
  _star_data.addStar(new Star(42, 0.272684f, 0.496f, 14.31f, 1.1f));
  _star_data.addStar(new Star(43, 0.291139f, 0.160759f, 17.91f, 0.67f));
  _star_data.addStar(new Star(44, 0.296203f, 0.137975f, 19.53f, 0.52f));
  _star_data.addStar(new Star(45, 0.296658f, 0.558937f, 13.7f, 1.24f));
  _star_data.addStar(new Star(46, 0.299266f, 0.761975f, 19.04f, 0.58f));
  _star_data.addStar(new Star(47, 0.306456f, 0.690709f, 14.12f, 1.17f));
  _star_data.addStar(new Star(48, 0.324051f, 0.186076f, 19.7f, 0.56f));
  _star_data.addStar(new Star(49, 0.343038f, 0.941772f, 21.89f, 0.82f));
  _star_data.addStar(new Star(50, 0.34638f, 0.0835443f, 19.01f, 0.51f));
  _star_data.addStar(new Star(51, 0.355038f, 0.137873f, 14.49f, 1.08f));
  _star_data.addStar(new Star(52, 0.36962f, 0.226582f, 16.85f, 0.69f));
  _star_data.addStar(new Star(53, 0.370557f, 0.769316f, 14.43f, 1.01f));
  _star_data.addStar(new Star(54, 0.382278f, 0.282278f, 17.31f, 0.73f));
  _star_data.addStar(new Star(55, 0.382962f, 0.320076f, 16.3f, 0.38f));
  _star_data.addStar(new Star(56, 0.387139f, 0.138835f, 16.46f, 0.83f));
  _star_data.addStar(new Star(57, 0.389873f, 0.65443f, 13.5f, 1.3f));
  _star_data.addStar(new Star(58, 0.422785f, 0.0594937f, 19.98f, 0.54f));
  _star_data.addStar(new Star(59, 0.443038f, 0.816456f, 17.12f, 0.73f));
  _star_data.addStar(new Star(60, 0.467089f, 0.0708861f, 13.7f, 0.25f));
  _star_data.addStar(new Star(61, 0.472506f, 0.904177f, 13.93f, 1.13f));
  _star_data.addStar(new Star(62, 0.486405f, 0.308152f, 16.09f, 0.42f));
  _star_data.addStar(new Star(63, 0.516076f, 0.104886f, 17.24f, 0.01f));
  _star_data.addStar(new Star(64, 0.531646f, 0.826582f, 18.29f, 0.67f));
  _star_data.addStar(new Star(65, 0.542203f, 0.71638f, 16.52f, 0.08f));
  _star_data.addStar(new Star(66, 0.546835f, 0.872152f, 17.43f, 0.69f));
  _star_data.addStar(new Star(67, 0.574684f, 0.864557f, 20.47f, 0.57f));
  _star_data.addStar(new Star(68, 0.575949f, 0.891139f, 21.73f, 0.79f));
  _star_data.addStar(new Star(69, 0.593089f, 0.172684f, 18.45f, 0.67f));
  _star_data.addStar(new Star(70, 0.613063f, 0.87881f, 15.42f, 0.85f));
  _star_data.addStar(new Star(71, 0.620253f, 0.153165f, 17.64f, 0.07f));
  _star_data.addStar(new Star(72, 0.622886f, 0.0898481f, 17.52f, 0.69f));
  _star_data.addStar(new Star(73, 0.672684f, 0.775899f, 16.61f, 0.71f));
  _star_data.addStar(new Star(74, 0.674684f, 0.946835f, 14.1f, 0.6f));
  _star_data.addStar(new Star(75, 0.689772f, 0.820152f, 17.46f, 0.69f));
  _star_data.addStar(new Star(76, 0.717165f, 0.342354f, 15.37f, 0.94f));
  _star_data.addStar(new Star(77, 0.730532f, 0.174532f, 14.62f, 1.06f));
  _star_data.addStar(new Star(78, 0.734076f, 0.237291f, 14.06f, 1.15f));
  _star_data.addStar(new Star(79, 0.735646f, 0.0731139f, 15.42f, 0.99f));
  _star_data.addStar(new Star(80, 0.742177f, 0.105646f, 16.19f, 0.72f));
  _star_data.addStar(new Star(81, 0.747241f, 0.729215f, 14.15f, 1.13f));
  _star_data.addStar(new Star(82, 0.75443f, 0.286101f, 16.22f, 0.89f));
  _star_data.addStar(new Star(83, 0.764456f, 0.188734f, 16.85f, 0.73f));
  _star_data.addStar(new Star(84, 0.818937f, 0.564911f, 18.04f, 0.71f));
  _star_data.addStar(new Star(85, 0.83476f, 0.965721f, 16.3f, 0.83f));
  _star_data.addStar(new Star(86, 0.86081f, 0.740937f, 18.62f, 0.53f));
  _star_data.addStar(new Star(87, 0.883544f, 0.5f, 18.79f, 0.6f));
  _star_data.addStar(new Star(88, 0.903797f, 0.846835f, 20.76f, 0.57f));
  _star_data.addStar(new Star(89, 0.925316f, 0.921519f, 21.72f, 0.72f));
  _star_data.addStar(new Star(90, 0.926582f, 0.355696f, 17.37f, 0.66f));
  _star_data.addStar(new Star(91, 0.929063f, 0.560051f, 13.22f, 1.36f));
  _star_data.addStar(new Star(92, 0.93319f, 0.42957f, 16.83f, 0.8f));
  _star_data.addStar(new Star(93, 0.933443f, 0.512886f, 17.82f, 0.66f));
  _star_data.addStar(new Star(94, 0.93438f, 0.32324f, 17.5f, 0.69f));
  _star_data.addStar(new Star(95, 0.937013f, 0.0826835f, 18.59f, 0.62f));
  _star_data.addStar(new Star(96, 0.943772f, 0.886076f, 14.73f, 1.01f));
  _star_data.addStar(new Star(97, 0.945392f, 0.840785f, 15.47f, 0.91f));
  _star_data.addStar(new Star(98, 0.946835f, 0.978481f, 17.59f, 0.66f));
  _star_data.addStar(new Star(99, 0.969646f, 0.959367f, 17.52f, 0.68f));
}

void HrDiagramLab::onSelectionChanged(bool selected, int star_id) {
  using namespace std;
  if (!selected || (star_id < 0) || (star_id > _star_data.size())) {
    ui->spinboxId->setValue(0);
    ui->spinboxMagnitude->setValue(0);
    ui->spinboxBV->setValue(0);
    ui->vmagSpinBox->setValue(0);
    ui->bmagSpinBox->setValue(0);
  } else {
    Star *star = _star_data.getStar(star_id);
    ui->spinboxId->setValue(star->_id);
    ui->spinboxMagnitude->setValue(star->_mag);
    ui->spinboxBV->setValue(star->_color);
    ui->vmagSpinBox->setValue(star->_mag);
    ui->bmagSpinBox->setValue(star->_mag + star->_color);
  }
}

/**
 * Fires when the Exit button is pressed
 */
void HrDiagramLab::on_exitButton_clicked(){

//  std::cout<<"ExpansionLab::on_exitButton_clicked "<<std::endl;

  // Just set the time to 0
  QApplication::quit();
}


int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  HRMainWindow w;
  w.setWindowTitle("Lab H: Hertzsprung-Russell diagram");
  w.showMaximized();
//    w.showFullScreen();

  return a.exec();
}
