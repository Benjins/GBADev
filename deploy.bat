@echo off

IF EXIST F:/ (
cp %1.gba F:/
RemoveDrive F
) ELSE (
echo "Could not copy, F:/ not inserted."
)