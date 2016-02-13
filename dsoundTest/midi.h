typedef struct{int start; unsigned short length; short pitch;} Note;typedef struct{Note* notes; int length;} Song;Note testSong_notes[] = {
{0, 2880, 13}, {2880, 2880, 15}, {5760, 2880, 13}, {8640, 2880, 12}, {11520, 2880, 13}, 
{14400, 2880, 15}, {17280, 2880, 13}, {20160, 2880, 12}, {23040, 2880, 13}, {25920, 2880, 13}, 
{28800, 2880, 14}, {31680, 2880, 12}, {34560, 2880, 13}, {37440, 2880, 12}, {40320, 2880, 13}, 
{0, 46080, 20}, {43200, 2880, 20}, };
Song testSong = {testSong_notes, 17};