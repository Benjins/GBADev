@echo off

arm-none-eabi-gcc -c main.c -mthumb-interwork -mthumb -O2 -o main.o
arm-none-eabi-gcc -c arm.c -mthumb-interwork -marm -O2 -o arm.o
arm-none-eabi-gcc main.o arm.o -mthumb-interwork -specs=gba.specs -o main.elf

arm-none-eabi-objcopy -v -O binary main.elf main.bin