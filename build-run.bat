@echo off

build.bat %1

echo Done building %1.

cp %1.gba ../emulators/ba-028/%1.gba
pushd .
cd ../emulators/ba-028
zip.exe Roms/%1.zip %1.gba
BoycottAdvance.exe Roms/%1.zip
popd
