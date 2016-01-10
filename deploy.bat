@echo off

IF EXIST E:/ (
cp %1.gba E:/
RemoveDrive E
) ELSE (
echo "Could not copy, E:/ not inserted."
)