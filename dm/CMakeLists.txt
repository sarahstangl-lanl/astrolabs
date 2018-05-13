# Dark Matter velocity curves lab

set(DM_SOURCE_FILES dm_gui.cpp dm_scene_graph.cpp)

if (WIN32)
    set(QWT_LIB  qwt)
else()
    set(QWT_LIB  qwt-qt5)

    # Yea this is a hack
    include_directories("/usr/include/qt5/qwt")
endif()


add_executable(astrolabs_dm ${DM_SOURCE_FILES} ${UIS_HDRS} ${COMMON_SOURCE_FILES})

target_link_libraries(astrolabs_dm Qt5::Widgets ${OPENGL_LIBRARIES} ${QWT_LIB})

install(TARGETS astrolabs_dm DESTINATION astrolabs)
