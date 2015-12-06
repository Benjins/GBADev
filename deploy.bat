@echo off

IF EXIST D:/ (
cp %1.gba D:/
RemoveDrive D
) ELSE (
echo "Could not copy, D:/ not inserted."
)