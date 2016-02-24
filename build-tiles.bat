@echo off

cl /Fetilemapper.exe toolsCode/Renderer.cpp toolsCode/windows_main.cpp tileMapper/main.cpp kernel32.lib user32.lib Gdi32.lib Comdlg32.lib

echo Done building tileMapper!

tilemapper.exe %1