gcc -std=c0x assets.c -o assets.out
./assets.out $1
gcc -std=c0x sounds.c -o sounds.out
./sounds.out $1
gcc -std=c0x midi.c -o midi.out
./midi.out $1

