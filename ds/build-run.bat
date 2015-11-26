@echo off

pushd %1
arm-none-eabi-g++ --std=c++11 -c -I"C:/devkitPro/libnds/include" main.cpp -march=armv5te -mtune=arm946e-s -fomit-frame-pointer -ffast-math -mthumb-interwork -mthumb -O2 -DARM9 -fno-rtti -fno-exceptions -o %1.o
popd
mv %1/%1.o %1.o
arm-none-eabi-g++ %1.o -mthumb-interwork -mthumb -L"C:/devkitPro/libnds/lib" -lnds9 -specs=ds_arm9.specs -o %1.elf
arm-none-eabi-objcopy -v -O binary %1.elf %1.arm9
ndstool -9 %1.arm9 -c %1.nds

echo Done building %1.

pushd .
cd ../../emulators/desmume
DeSmuME_0.9.10_x86.exe ../../DevEnviron/ds/%1.nds
popd

