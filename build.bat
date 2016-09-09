@echo off

pushd %1
arm-none-eabi-g++ --std=c++11 -c main.c -mthumb-interwork -mthumb -O2 -o %1.o
if exist armCode.c (
echo Building ARM code for %1
arm-none-eabi-g++ --std=c++11 -c armCode.c -marm -O2 -o %1_arm.o
mv %1_arm.o ..
)
popd
mv %1/%1.o %1.o
if exist %1_arm.o (
echo Using default ARM code for %1
) else (
echo Using custom ARM code for %1
arm-none-eabi-g++ --std=c++11 -c armCode.c -marm -O2 -o %1_arm.o
)

arm-none-eabi-gcc %1.o %1_arm.o -mthumb-interwork -mthumb -specs=gba.specs -o %1.elf

arm-none-eabi-objcopy -v -O binary %1.elf %1.gba
gbafix %1.gba