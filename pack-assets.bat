@echo off
gcc -std=c11 assets.c -o assets.exe
echo Built assets packer.
assets.exe %1
echo Packed assets.
