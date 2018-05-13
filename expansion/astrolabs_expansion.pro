#-------------------------------------------------
#
# Project created by QtCreator 2013-06-17T17:31:45
#
#-------------------------------------------------

QT       += core gui

CONFIG += release

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

TARGET = astrolabs_kepler
TEMPLATE = app

INCLUDEPATH += ../include ../

SOURCES += expansion_gui.cpp \
        expansion_scene.cpp


HEADERS  += expansion_gui.h \
        expansion_scene.h

LIBS += -lqwt-qt5

FORMS    += expansion.ui
RESOURCES += ../resources.qrc
RC_FILE = expansion.rc
