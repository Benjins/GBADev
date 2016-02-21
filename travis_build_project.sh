wd=`pwd`
ARMBIN="$wd/devkitpro/devkitARM/bin"
"$ARMBIN/arm-none-eabi-g++" --std=c++11 "-I$ARMBIN/../../libgba/include" -c $1/main.c -mthumb-interwork -mthumb -O2 -o $1.o
"$ARMBIN/arm-none-eabi-gcc" $1.o "-L$ARMBIN/../../libgba/lib" -lgba -mthumb-interwork -mthumb -specs=gba.specs -o $1.elf
"$ARMBIN/arm-none-eabi-objcopy" -v -O binary $1.elf $1.gba
"$ARMBIN/gbafix" $1.gba
