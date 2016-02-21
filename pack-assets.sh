gcc -std=c11 assets.c -o assets.out
./assets.out $1
gcc -std=c11 sounds.c -o sounds.out
./sounds.out $1
gcc -std=c11 midi.c -o midi.out
./midi.out $1

