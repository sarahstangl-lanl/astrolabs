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

INCLUDEPATH += ../include \
               ../contrib/include

#LIBPATH += /home/stou/local/lib64

SOURCES += kepler_scene_graph.cpp \
           kepler_gui.cpp \
           ../contrib/src/glew.cpp


HEADERS  += kepler_scene_graph.h \
            kepler_gui.h

FORMS    += kepler.ui
RESOURCES += ../resources.qrc
RC_FILE = kepler.rc

DISTFILES += kepler.rc
