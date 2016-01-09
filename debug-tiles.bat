@echo off

cl /Fetilemapper_dbg.exe /Od /Zi tileMapper/*.cpp kernel32.lib user32.lib Gdi32.lib

::devenv tilemapper_dbg.exe