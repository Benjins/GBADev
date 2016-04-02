@echo off
gcc -Wall -std=c11 assets.c -o assets.exe
echo Built assets packer.
assets.exe %1
echo Packed assets.

gcc -Wall -std=c11 sounds.c -o sounds.exe
echo Built sounds packer
sounds.exe %1
echo Packed Sounds

gcc -Wall -std=c11 midi.c -o midi.exe
echo Built Midi packer
midi.exe %1
echo Packed Midi

gcc -Wall -std=c11 vid.c -o vid.exe
echo Built vid packer
vid.exe %1
echo Packed Video

