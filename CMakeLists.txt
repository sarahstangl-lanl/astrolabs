cmake_minimum_required(VERSION 3.5)
project(astrolabs)

#TODO: Build RPM
#TODO: Option to use GLEW
#TODO: Set assets path

# Platform specific crap
if (WIN32)
    set (CMAKE_PREFIX_PATH "C:\\Qt\\5.7\\msvc2015_64")
    set (CMAKE_INSTALL_PREFIX "\\\\Program Files\\\\Astrolabs")
    include_directories("C:\\Qwt-6.1.3\\include")
    link_directories("C:\\Qwt-6.1.3\\lib")
else()
    # Need C++ 2011 support and -fPIC for Qt5
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fPIC -Wall")
    set(CPACK_GENERATOR "ZIP")
endif()

find_package(OpenGL REQUIRED)

# Setup Qt
#find_package(Qt5Widgets Qt5OpenGL REQUIRED)
find_package(Qt5Widgets COMPONENTS QtCore QtGui QtOpenGL REQUIRED)
set(QT_USE_QTOPENGL TRUE)

include_directories(${Qt5Widgets_INCLUDE_DIRS})

# Automatically run moc
set(CMAKE_AUTOMOC ON)

# Automatically run resource compiler
set(CMAKE_AUTORCC ON)

# Automatically run UI thing
set(CMAKE_AUTOUIC ON)


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories(".")
include_directories("include")
include_directories("contrib/include")

set (COMMON_SOURCE_FILES "../contrib/src/glew.c" "../resources.qrc")

# We want to run from the top level directory so as to use the assets/ folder
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Include the different labs
add_subdirectory(dm)
add_subdirectory(expansion)
add_subdirectory(hr)
add_subdirectory(kepler)

# The assets directory
install(DIRECTORY "${PROJECT_SOURCE_DIR}/assets/" DESTINATION "astrolabs/assets/")

# Redistributable files (Windows Only)
install(DIRECTORY "${PROJECT_SOURCE_DIR}/redist/" DESTINATION "astrolabs/")

# Setup package (RPM)

set(CPACK_GENERATOR "ZIP")

set(CPACK_PACKAGE_VERSION "2017.05.10")
set(CPACK_PACKAGE_RELEASE 1)
SET(CPACK_PACKAGE_VERSION_MAJOR "2017")
SET(CPACK_PACKAGE_VERSION_MINOR "5")
SET(CPACK_PACKAGE_VERSION_PATCH "3")

set(CPACK_PACKAGE_NAME "astrolabs")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Astronomy 1001 Lab Software")
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
#SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/Copyright.txt")

set(CPACK_PACKAGE_CONTACT "Stou Sandalski")
set(CPACK_PACKAGE_VENDOR "Minnesota Institute for Astrophysics")

set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_PACKAGE_RELEASE}.${CMAKE_SYSTEM_PROCESSOR}")
include(CPack)

SET(CPACK_PACKAGE_INSTALL_DIRECTORY "CMake ${CMake_VERSION_MAJOR}.${CMake_VERSION_MINOR}")
if (WIN32)
    SET(CPACK_SET_DESTDIR 0)
    # There is a bug in NSI that does not handle full unix paths properly. Make
    # sure there is at least one set of four (4) backlasshes.
#    SET(CPACK_PACKAGE_ICON "${CMake_SOURCE_DIR}\\\\assets\\\\icons\\\\installer.bmp")
#    SET(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\MyExecutable.exe")
    SET(CPACK_NSIS_DISPLAY_NAME "${CPACK_PACKAGE_INSTALL_DIRECTORY} Astro 1001 Programs")
#    SET(CPACK_NSIS_HELP_LINK "http:\\\\\\\\www.my-project-home-page.org")
#    SET(CPACK_NSIS_URL_INFO_ABOUT "http:\\\\\\\\www.my-personal-home-page.com")
#    SET(CPACK_NSIS_CONTACT "sandalski@astro.umn.edu")
#    SET(CPACK_NSIS_MODIFY_PATH ON)
else()
#    SET(CPACK_STRIP_FILES "bin/MyExecutable")
#    SET(CPACK_SOURCE_STRIP_FILES "")
endif()
#SET(CPACK_PACKAGE_EXECUTABLES "MyExecutable" "My Executable")

# https://cmake.org/Wiki/CMake:Component_Install_With_CPack#CPack_Generator_specific_behavior


