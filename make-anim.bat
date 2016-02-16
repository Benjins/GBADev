@echo off

cl /Feanimer.exe animer/*.cpp kernel32.lib user32.lib Gdi32.lib Comdlg32.lib

echo Done building animer!

animer.exe %1