# HR diagram build file

set(HR_SOURCE_FILES hr_main.cpp star_image.cpp)

add_executable(astrolabs_hr ${HR_SOURCE_FILES} ${UIS_HDRS} ../resources.qrc)

target_link_libraries(astrolabs_hr Qt5::Widgets)

install(TARGETS astrolabs_hr DESTINATION astrolabs)