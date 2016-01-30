@echo off

pushd %1
arm-none-eabi-g++ --std=c++11 -I../../libgba/include -I../../gbaTonc/include -c main.c -mthumb-interwork -mthumb -O2 -o %1.o
popd
mv %1/%1.o %1.o
arm-none-eabi-gcc %1.o -L../libgba/lib -L../gbaTonc -ltonc -lgba -mthumb-interwork -mthumb -specs=gba.specs -o %1.elf
arm-none-eabi-objcopy -v -O binary %1.elf %1.gba
gbafix %1.gba

echo Done building %1.

cp %1.gba ../emulators/VizBoyAdv/%1.gba
pushd .
cd ../emulators/VizBoyAdv
VisualBoyAdvance.exe %1.gba
popd
