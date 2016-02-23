gcc assets.c -o assets.out
./assets.out $1
gcc sounds.c -o sounds.out
./sounds.out $1
gcc midi.c -o midi.out
./midi.out $1

