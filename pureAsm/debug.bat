::@echo off

call build-asm.bat

asm.exe

cp test.gba ../../emulators/ba-028/test.gba
pushd .
cd ../../emulators/ba-028
zip.exe Roms/test.zip test.gba
BoycottAdvance.exe Roms/test.zip
popd
