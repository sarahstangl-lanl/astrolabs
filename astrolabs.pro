#-------------------------------------------------
#
# Project created by QtCreator 2013-06-17T17:31:45
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets opengl

CONFIG   += release
TARGET = astrolabs
TEMPLATE = app

INCLUDEPATH += ./hr \
            ./kepler \
            ./lensing \
            ./dm

SOURCES += main.cpp \
        mainwindow.cpp \
    dm/dark_matter_lab.cpp \
    dm/dm_scene_graph.cpp \
    dm/radial_velocity_handler.cpp \
    kepler/lab_kepler.cpp \
    kepler/kepler_node_helpers.cpp \
    kepler/kepler_scene_graph.cpp \
    hr/star_image.cpp \
    hr/lab_hrdiagram.cpp\
    welcome_screen.cpp \
    lensing/lensing_lab.cpp \
    lensing/lensed_image.cpp \
    dm/dm_nodekit.cpp

HEADERS  += mainwindow.h \
    dm/dark_matter_lab.h \
    dm/dm_scene_graph.h \
    dm/graph_pulses.h \
    dm/radial_velocity_handler.h \
    dm/world_scene.h \
    hr/star_data.h \
    hr/star_image.h \
    hr/star.h \
    hr/lab_hrdiagram.h \
    kepler/lab_kepler.h \
    kepler/kepler_node_helpers.h \
    kepler/kepler_scene_graph.h \
    welcome_screen.h \
    lensing/lensing_lab.h \
    lensing/lensed_image.h \
    dm/dm_nodekit.h

LIBS += -losg\
        -losgDB\
        -losgFX\
        -losgGA\
        -losgQt\
        -losgViewer\
        -losgWidget\
        -losgParticle\
        -losgUtil

FORMS    += mainwindow.ui \
    kepler/lab_kepler.ui \
    hr/lab_hrdiagram.ui \
    welcome_screen.ui \
    dm/dark_matter_lab.ui \
    lensing/lensing_lab.ui

unix{
    CONFIG += link_pkgconfig
    PKGCONFIG += qwt
}

win32{
    LIBS += -lOpenThreads\
            -lqwt

}

RESOURCES += resources.qrc
