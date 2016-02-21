g++ -std=c++0x assets.c -o assets.out
./assets.out $1
g++ -std=c++0x sounds.c -o sounds.out
./sounds.out $1
g++ -std=c++0x midi.c -o midi.out
./midi.out $1

