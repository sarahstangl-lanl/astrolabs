# Astrolabs #

This is the code for the UMN Astronomy 1001 Labs. 

# Dependencies #

Uses Qt 5, Qwt and OpenGL 3.3.


# Code Structure #

The code for each lab is inside its own sub-directory (expansion, dm, hr, kepler) and is built as a separate
application. For labs having a graphics component the source is roughly split between 
Qt GUI code (`_gui.cpp/.hpp` ) and OpenGL graphics code (`_scene_graph.cpp/.hpp` ).


`assets` - Files that need to be distributed with the applications (shaders, backgrounds, etc.)
`config` - Configuration files for live CD spins
`contrib` - External dependencies (e.g. GL Extension Wrangler or GLEW) that have been included here for simplicity.
 `dm` - Dark Matter Lab sources
`expansion` - Expansion Lab
`hr` - HR Lab
`include` - Common include files 
`kepler` - Kepler Lab


# Building

Dependencies: Qt5, Qwt, OpenGL 3.3



## Linux (Fedora )

Install dependencies

```
dnf install qt-creator qwt-qt5-devel libpng-devel libjpeg-devel
```

Build programs

```sh
$ git clone https://bitbucket.org/stou/astrolabs
$ mkdir astrolabs/build
$ cd astrolabs/build
$ cmake ../
$ make
```

## Windows (64 bit) ##


### Install Dependencies

1. Install hg client ( http://tortoisehg.bitbucket.org/de/download/index.html )

2. Install CMake https://cmake.org/download/

3. Install Visual Studio https://www.visualstudio.com/vs/community/

4. Download Qt from https://www.qt.io/download-open-source/
  - Select Qt 5.7 msvc2015 x64

5. Install qwt:

http://www.qtcentre.org/threads/53787-HowTo-Installation-of-Qt-5-0-1-and-Qwt-6-1-0-rc3-(Win7-64bit)

Add C:\Qt\5.7\msvc2015_64\bin to system path


### Build

1. Clone Repo: ```https://bitbucket.org/stou/astrolabs```

2. Open "Developer Command Prompt"

```
cd C:\Program Files (x86)\Microsoft Visual Studio 14.0>
VC\vcvarsall amd64
mkdir ASTROLABS_BUILD_DIRECTORY
cd ASTROLABS_BUILD_DIRECTORY
cmake ../astrolabs/ -G "NMake Makefiles"
nmake
cpack
```

To rebuild just 

```
VC\vcvarsall amd64
cd C:\Users\Stou\Desktop\astrolabs_build_release
nmake
cpack
```
On Windows the packager will copy the contents of astrolabs/redist/ into the root directory of the zip. 


The instructions are for building with Visual Studio 2012. Double check that there are no binaries for libpng, etc. for 
your environment before building from scratch (it's very time consuming). There seems to be plenty of binaries for Visual Studio 2010

Download and build:
1. Visual Studio 2012
2. Windows SDK
3. Qt Creator
4. CMake
5. libjpeg
6. libpng
7. zlib
6. QWT ( follow instructions here file:///C:/Users/Stou/projects/qwt-6.1.0/doc/html/qwtinstall.html )
7. Edit build.bat and adjust paths correctly
8. Run build.sh to build all of the sub programs (won't be able to build individual projects from qt creator)


# Deploy

## Windows:

Run:

```
windeployqt .
```

to create the required Qt directories (`translations`, `platforms`, `iconengines`, `imageformats`)

and make sure that the above DLLs are also present: 

```
vcruntime140.dll
ucrtbase.dll (x86_64)
Qt5Core.dll
Qt5Gui.dll
Qt5OpenGL.dll
Qt5PrintingSupport.dll
Qt5Svg.dll
Qt5Widgets.dll
qwt.dll
```

anything placed inside `./redist` will be copied into the main package by CPack.


# Development Notes #

Qt Signals and Slots
http://qt-project.org/wiki/Qt_for_beginners_Signals_and_slots


`.ui` files contain Qt Creator form information... through some QT magic this is connected to the driver code in 
`_main.cpp/_main.h` for each lab.  Qt classes are special due to the slots and signals mechanism so they require
the `Q_OBJECT` macro and the `moc` compiler to be run first. CMake takes care of all the magic in the background. 


# External Contributions#

See LICENSE.md for the code license. External things used:

* Fugue Icons

Copyright (C) 2010 Yusuke Kamiyamane. All rights reserved.
The icons are licensed under a Creative Commons Attribution
3.0 license. <http://creativecommons.org/licenses/by/3.0/>

* Planet Textures

 http://planetpixelemporium.com/planets.html

* Eta-car model

https://www.eso.org/public/usa/products/models3d/3dmodel_004/

Credit
NASA's Goddard Space Flight Center/Ed Campion & UNAM/W. Steffen

* Material Icons

Copyright (c) 2016 Google Inc.
Apache 2.0












https://cmake.org/Wiki/CMake:CPackPackageGenerators#RPM_.28Unix_Only.29

## Linux Live CD ##



https://fedoraproject.org/wiki/How_to_create_and_use_a_Live_CD?rd=How_to_create_and_use_Fedora_Live_CD

Live CD kickstart configuration is in ./config


```bash
livecd-creator --verbose --config=./astro_1001_kiosk.cfg --fslabel=Astro-Labs --cache=/var/cache/live

```



# Modifying the Code #

`.ui` files contain UI definitions... edit with Qt Creator

Stylesheet info: http://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qspinbox


# Known Issues

## Expansion

* Names for the expansion programs: Virtual Telescope and ??

## Kepler:

* If you select the text in the Angle / Velocity box it stays selected.
* Sweep / Other button touch interaction 
* For Windows under Pen and Touch inside Touch disable Press and Hold gesture.
* New Orbit icon should change to an X 
* See if icons can be bigger
* Question: Should animations be slower?


## Dark Matter:

* Can we drag the plot line?
* Galaxy Cluster... the peeks are too narrow and get weird, something with the sampling. Either increase sampling # OR widen the peaks.
* Lensed image gets clipped
* Contours on model-image in legend are too aliased, should draw circle directly on legend









