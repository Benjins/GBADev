@echo off

call build.bat %1

echo Done building %1.

cp %1.gba ../emulators/nosGba/%1.gba
pushd .
cd ../emulators/nosGba
NOSGBA.exe "C:\devkitPro\emulators\nosGba\%1.gba"
popd
