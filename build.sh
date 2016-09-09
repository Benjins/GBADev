arm-none-eabi-g++ --std=c++11 -c $1/main.c -mthumb-interwork -mthumb -O2 -o $1.o
if [ -e $1/armCode.c ] ; then
arm-none-eabi-g++ --std=c++11 -c $1/armCode.c -marm -O2 -o $1.Arm.o
else
arm-none-eabi-g++ --std=c++11 -c armCode.c -marm -O2 -o $1.Arm.o
fi
arm-none-eabi-gcc $1.o $1.Arm.o -mthumb-interwork -mthumb -specs=gba.specs -o $1.elf
arm-none-eabi-objcopy -v -O binary $1.elf $1.gba
gbafix $1.gba
