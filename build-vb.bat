@echo off

build.bat %1

echo Done building %1.

cp %1.gba ../emulators/VizBoyAdv/%1.gba
pushd .
cd ../emulators/VizBoyAdv
VisualBoyAdvance.exe %1.gba
popd
