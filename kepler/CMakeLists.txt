# Kepler lab

set(KEPLER_SOURCE_FILES kepler_gui.cpp kepler_scene_graph.cpp)

add_executable(astrolabs_kepler ${KEPLER_SOURCE_FILES} ${UIS_HDRS} ${COMMON_SOURCE_FILES})

target_link_libraries(astrolabs_kepler Qt5::Widgets ${OPENGL_LIBRARIES})

install(TARGETS astrolabs_kepler DESTINATION astrolabs)