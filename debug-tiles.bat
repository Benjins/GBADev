@echo off

cl /Fetilemapper_dbg.exe /Od /Zi tileMapper/Renderer.cpp tileMapper/windows_main.cpp tileMapper/main.cpp kernel32.lib user32.lib Gdi32.lib Comdlg32.lib

::devenv tilemapper_dbg.exe