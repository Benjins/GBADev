typedef struct{int start; short length; short pitch;} Note;typedef struct{Note* notes; int length;} Song;Note testSong_notes[] = {
{0, 1440, 13}, {0, 1440, 13}, {1440, 1440, 15}, {1440, 1440, 15}, {2880, 1440, 13}, 
{4320, 1440, 15}, {5760, 1440, 13}, {5760, 1440, 13}, {7200, 1440, 15}, {7200, 1440, 15}, 
{8640, 1440, 13}, {10080, 1440, 15}, {12960, 4320, 16}, {17280, 1440, 18}, {20160, 1440, 18}, 
{21600, 4320, 13}, {25920, 7200, 15}, {33120, 12960, 11}, {46080, 1440, 15}, {47520, 1440, 14}, 
{48960, 1440, 13}, {50400, 2880, 13}, {53280, 1440, 13}, {54720, 2880, 12}, };
Song testSong = {testSong_notes, 24};