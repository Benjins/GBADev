gcc -std=c99 -Wall assets.c -o assets.out
./assets.out $1
gcc -std=c99 -Wall sounds.c -o sounds.out
./sounds.out $1
gcc -std=c99 -Wall midi.c -o midi.out
./midi.out $1

