@echo off

cl /Fedbg_animer.exe /Od /Zi animer/*.cpp kernel32.lib user32.lib Gdi32.lib Comdlg32.lib

::devenv dbg_animer.exe