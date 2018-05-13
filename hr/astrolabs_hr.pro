#-------------------------------------------------
#
# Project created by QtCreator 2013-06-17T17:31:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = astrolabs_hr
TEMPLATE = app

INCLUDEPATH += ../

SOURCES += hr_main.cpp \
    star_image.cpp \
    lab_hrdiagram.cpp

HEADERS  += hr_main.h\
    star_data.h \
    star_image.h \
    star.h \
    lab_hrdiagram.h


FORMS    += lab_hrdiagram.ui

unix: CONFIG += link_pkgconfig

RESOURCES += ../resources.qrc
RC_FILE = hr.rc
