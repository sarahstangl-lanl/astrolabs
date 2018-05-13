#-------------------------------------------------
#
# Project created by QtCreator 2013-06-17T17:31:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

CONFIG += release

TARGET = astrolabs_dm
TEMPLATE = app

SOURCES += dm_gui.cpp \
           dm_scene_graph.cpp


HEADERS  += dm_gui.h \
         dm_scene_graph.h

FORMS    += dark_matter_lab.ui

CONFIG += Qt5Qwt6

unix{
    CONFIG += link_pkgconfig
    PKGCONFIG += Qt5Qwt6
}

RESOURCES += ../resources.qrc
RC_FILE = dm.rc
