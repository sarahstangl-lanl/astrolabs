# Kepler lab

set(KEPLER_SOURCE_FILES expansion_gui.cpp expansion_scene.cpp)

add_executable(astrolabs_expansion ${KEPLER_SOURCE_FILES} ${UIS_HDRS} ${COMMON_SOURCE_FILES})

target_link_libraries(astrolabs_expansion Qt5::Widgets ${OPENGL_LIBRARIES})

install(TARGETS astrolabs_expansion DESTINATION astrolabs)