@echo off

cl /Fetilemapper.exe tileMapper/*.cpp kernel32.lib user32.lib Gdi32.lib

echo Done building tileMapper!

tilemapper.exe topDown