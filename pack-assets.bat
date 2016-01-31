@echo off
gcc -std=c11 assets.c -o assets.exe
echo Built assets packer.
assets.exe %1
echo Packed assets.

gcc -std=c11 sounds.c -o sounds.exe
echo Built sounds packer
sounds.exe %1
echo Packed Sounds
