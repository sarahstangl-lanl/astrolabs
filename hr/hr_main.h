#ifndef HR_MAIN_H
#define HR_MAIN_H

#include <QtWidgets/QBoxLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>

#include "star_image.h"

namespace Ui {
  class HrDiagramLab;
}

// TODO: These two classes can be combined now that the combined launcher is dead
class HrDiagramLab : public QWidget {
  Q_OBJECT

public:
  explicit HrDiagramLab(QWidget *parent = 0);

  ~HrDiagramLab();

  void addStars();

public
  slots:

  void onSelectionChanged(bool selected, int star_id);

  void on_exitButton_clicked();

private:
  Ui::HrDiagramLab *ui;
  StarData _star_data;
};


class HRMainWindow : public QMainWindow {
  Q_OBJECT

public:
  HRMainWindow(QWidget *parent = 0) : QMainWindow(parent) {
    QWidget *widget = new QWidget();
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);

    widget->setLayout(layout);
    layout->addWidget(new HrDiagramLab(this));


    setCentralWidget(widget);
  }
};

#endif // HR_MAIN_H
