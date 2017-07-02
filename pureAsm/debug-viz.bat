::@echo off

call build-asm.bat

asm.exe

cp test.gba ../../emulators/VizBoyAdv/test.gba
pushd .
cd ../../emulators/VizBoyAdv
VisualBoyAdvance.exe test.gba
popd
