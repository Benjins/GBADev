arm-none-eabi-g++ --std=c++11 "-I$DEVKITPRO/libgba/include" -c $1/main.c -mthumb-interwork -mthumb -O2 -o $1.o
arm-none-eabi-gcc $1.o "-L$DEVKITPRO/libgba/lib" -lgba -mthumb-interwork -mthumb -specs=gba.specs -o $1.elf
arm-none-eabi-objcopy -v -O binary $1.elf $1.gba
gbafix $1.gba

vba $1.gba
