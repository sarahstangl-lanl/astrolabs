REM Use VS2012 x64 Cross Tools Command Prompt

set PATH=C:\Qt\Qt5.1.0\5.1.0\msvc2012_64_opengl\bin;%PATH%
set INCLUDE=%INCLUDE%;C:\Program Files\OpenSceneGraph\include;C:\Qwt-6.1.0\include;C:\Users\Stou\projects\astrolabs
set LIB=%LIB%;C:\Program Files\OpenSceneGraph\lib;C:\Qwt-6.1.0\lib

if not exist build_batch mkdir build_batch
if not exist deployment mkdir deployment
cd deployment
if not exist textures mkdir textures
cd ..

cd build_batch
qmake ..\dm\astrolabs_dm.pro -o dm_make
nmake /f dm_make 

qmake ..\hr\astrolabs_hr.pro -o hr_make
nmake /f hr_make

qmake ..\kepler\astrolabs_kepler.pro -o kepler_make
nmake /f kepler_make

qmake ..\lensing\astrolabs_lensing.pro -o lensing_make
nmake /f lensing_make

copy /Y release\*.exe ..\deployment\
copy /Y release\*.exe E:\Downloads\astrowindows\deployment\
copy ..\assets\textures\*.* ..\deployment\textures\
copy ..\assets\textures\*.* E:\Downloads\astrowindows\deployment\textures\
cd ..