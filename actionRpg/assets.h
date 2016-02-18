typedef struct{int type; int width; int height; unsigned short* data;} Sprite;
 	 typedef struct{Sprite map; Sprite* bgSprites; int* spriteFlags; int bgCount;} Background;
static unsigned short aFont_data[] = {
0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,};
static Sprite aFont = {0, 8, 8, aFont_data
};
static unsigned short bFont_data[] = {
0,1,1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,1,0,0,};
static Sprite bFont = {0, 8, 8, bFont_data
};
static unsigned short cFont_data[] = {
0,0,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,};
static Sprite cFont = {0, 8, 8, cFont_data
};
static unsigned short dFont_data[] = {
0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,0,0,0,0,};
static Sprite dFont = {0, 8, 8, dFont_data
};
static unsigned short eFont_data[] = {
0,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,};
static Sprite eFont = {0, 8, 8, eFont_data
};
static unsigned short fFont_data[] = {
0,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,};
static Sprite fFont = {0, 8, 8, fFont_data
};
static unsigned short gFont_data[] = {
0,0,1,1,1,1,1,0,0,1,1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,1,1,1,1,0,};
static Sprite gFont = {0, 8, 8, gFont_data
};
static unsigned short hFont_data[] = {
0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,};
static Sprite hFont = {0, 8, 8, hFont_data
};
static unsigned short iFont_data[] = {
0,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,1,1,0,};
static Sprite iFont = {0, 8, 8, iFont_data
};
static unsigned short jFont_data[] = {
0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,};
static Sprite jFont = {0, 8, 8, jFont_data
};
static unsigned short kFont_data[] = {
0,1,0,0,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,1,0,};
static Sprite kFont = {0, 8, 8, kFont_data
};
static unsigned short lFont_data[] = {
0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,};
static Sprite lFont = {0, 8, 8, lFont_data
};
static unsigned short mFont_data[] = {
0,1,0,0,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,};
static Sprite mFont = {0, 8, 8, mFont_data
};
static unsigned short nFont_data[] = {
0,1,1,0,0,0,1,0,0,1,1,0,0,0,1,0,0,1,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,0,1,0,};
static Sprite nFont = {0, 8, 8, nFont_data
};
static unsigned short oFont_data[] = {
0,0,1,1,1,1,0,0,0,1,1,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,1,0,0,0,1,1,1,1,0,0,};
static Sprite oFont = {0, 8, 8, oFont_data
};
static unsigned short pFont_data[] = {
0,1,1,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,};
static Sprite pFont = {0, 8, 8, pFont_data
};
static unsigned short qFont_data[] = {
0,0,1,1,1,1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0,1,1,0,0,1,0,0,0,0,1,0,0,1,0,1,0,0,1,0,0,1,0,0,1,0,1,0,0,0,1,1,1,1,0,0,0,0,0,0,0,1,1,0,};
static Sprite qFont = {0, 8, 8, qFont_data
};
static unsigned short rFont_data[] = {
0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,1,1,0,0,1,1,1,1,1,0,0,0,1,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,1,0,};
static Sprite rFont = {0, 8, 8, rFont_data
};
static unsigned short sFont_data[] = {
0,0,1,1,1,1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,0,0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,0,1,0,0,0,1,1,0,1,1,0,0,0,0,1,1,1,0,0,0,};
static Sprite sFont = {0, 8, 8, sFont_data
};
static unsigned short tFont_data[] = {
0,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,};
static Sprite tFont = {0, 8, 8, tFont_data
};
static unsigned short uFont_data[] = {
0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,1,0,0,0,1,0,0,1,0,0,0,0,1,1,1,1,0,0,};
static Sprite uFont = {0, 8, 8, uFont_data
};
static unsigned short vFont_data[] = {
0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,1,0,0,1,1,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,};
static Sprite vFont = {0, 8, 8, vFont_data
};
static unsigned short wFont_data[] = {
1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,1,1,1,1,0,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,1,1,1,1,1,0,0,0,1,1,1,1,0,0,0,0,1,0,0,1,0,0,};
static Sprite wFont = {0, 8, 8, wFont_data
};
static unsigned short xFont_data[] = {
0,1,0,0,0,0,1,0,0,1,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,0,1,1,0,0,0,1,1,0,0,1,0,0,0,1,0,0,0,1,1,0,};
static Sprite xFont = {0, 8, 8, xFont_data
};
static unsigned short yFont_data[] = {
0,1,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,};
static Sprite yFont = {0, 8, 8, yFont_data
};
static unsigned short zFont_data[] = {
0,1,1,1,1,1,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,};
static Sprite zFont = {0, 8, 8, zFont_data
};
static unsigned short textBoxSprite_data[] = {
2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};
static Sprite textBoxSprite = {0, 16, 16, textBoxSprite_data
};
static unsigned short monsterSprite_data[] = {
0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0,3,3,4,3,3,0,0,0,3,4,4,4,3,0,0,0,3,4,4,4,3,0,0,3,3,3,3,3,3,0,3,3,3,3,3,3,3,3,3,3,0,0,0,0,3,3,};
static Sprite monsterSprite = {0, 8, 8, monsterSprite_data
};
static unsigned short playerSpriteUp_data[] = {
0,0,4,4,4,0,0,0,0,4,4,4,4,4,0,0,0,4,4,4,4,4,0,0,0,6,6,6,6,6,0,0,6,6,6,6,6,6,6,0,0,0,5,5,5,0,0,0,0,0,5,0,5,0,0,0,0,5,5,0,5,5,0,0,};
static Sprite playerSpriteUp = {0, 8, 8, playerSpriteUp_data
};
static unsigned short playerSpriteDown_data[] = {
0,0,4,4,4,0,0,0,0,4,4,6,4,4,0,0,0,4,6,6,6,4,0,0,0,6,6,6,6,6,0,0,6,6,6,6,6,6,6,0,0,0,5,5,5,0,0,0,0,0,5,0,5,0,0,0,0,5,5,0,5,5,0,0,};
static Sprite playerSpriteDown = {0, 8, 8, playerSpriteDown_data
};
static unsigned short playerSpriteLeft_data[] = {
0,0,4,4,4,0,0,0,0,0,6,6,4,4,0,0,0,0,6,6,6,4,0,0,0,0,6,6,6,0,0,0,0,0,6,6,6,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,0,0,0,0,0,0,5,5,0,0,0,0,};
static Sprite playerSpriteLeft = {0, 8, 8, playerSpriteLeft_data
};
static unsigned short playerSpriteRight_data[] = {
0,0,4,4,4,0,0,0,0,4,4,6,6,0,0,0,0,4,6,6,6,0,0,0,0,0,6,6,6,0,0,0,0,0,6,6,6,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,0,0,0,0,0,0,0,5,5,0,0,0,};
static Sprite playerSpriteRight = {0, 8, 8, playerSpriteRight_data
};
static unsigned short backMap_bg0_data[] = {
3,7,3,7,7,3,7,7,7,7,7,7,7,7,8,7,7,7,8,7,7,3,7,8,7,8,7,8,8,7,8,8,7,8,7,7,8,7,7,7,7,8,8,7,8,7,7,8,7,7,7,8,8,7,7,8,7,3,7,7,7,7,7,7,};
static Sprite backMap_bg0 = {0, 8, 8, backMap_bg0_data
};
static unsigned short backMap_bg1_data[] = {
10,9,9,9,9,9,9,9,9,9,9,10,10,10,9,9,9,9,9,9,9,9,10,10,9,10,9,10,9,9,9,10,9,10,9,9,10,9,9,10,9,9,10,9,9,10,10,9,9,9,10,9,10,9,9,9,9,9,9,10,10,9,9,10,};
static Sprite backMap_bg1 = {0, 8, 8, backMap_bg1_data
};
static unsigned short backMap_bg2_data[] = {
6,6,11,11,11,6,11,11,6,6,6,6,11,6,11,11,11,6,11,6,6,6,6,11,11,6,6,11,6,6,6,6,11,11,6,11,6,11,6,6,6,11,6,11,6,11,6,6,6,6,11,6,11,11,11,6,11,11,11,11,11,6,11,11,};
static Sprite backMap_bg2 = {0, 8, 8, backMap_bg2_data
};
static unsigned short backMap_bg3_data[] = {
0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,};
static Sprite backMap_bg3 = {0, 8, 8, backMap_bg3_data
};
static unsigned short backMap_bg4_data[] = {
0,1,1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,1,0,0,};
static Sprite backMap_bg4 = {0, 8, 8, backMap_bg4_data
};
static unsigned short backMap_bg5_data[] = {
0,0,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,};
static Sprite backMap_bg5 = {0, 8, 8, backMap_bg5_data
};
static unsigned short backMap_bg6_data[] = {
0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,0,0,0,0,};
static Sprite backMap_bg6 = {0, 8, 8, backMap_bg6_data
};
static unsigned short backMap_bg7_data[] = {
12,12,9,9,9,9,9,12,9,9,9,9,9,9,9,9,9,9,9,9,9,12,12,12,9,9,9,12,12,9,12,9,9,12,9,9,9,9,12,12,9,12,12,9,9,9,9,9,9,12,12,12,9,12,12,12,9,9,9,9,9,9,9,12,};
static Sprite backMap_bg7 = {0, 8, 8, backMap_bg7_data
};
static unsigned short backMap_bg8_data[] = {
12,12,9,9,9,9,9,12,9,9,9,5,9,9,9,9,9,9,5,13,5,12,12,12,9,9,9,5,12,9,12,9,9,12,9,9,9,9,12,12,9,12,12,9,9,9,9,9,9,12,12,12,9,12,12,12,9,9,9,9,9,9,9,12,};
static Sprite backMap_bg8 = {0, 8, 8, backMap_bg8_data
};
static unsigned short backMap_bg9_data[] = {
12,14,14,14,14,14,9,12,9,14,14,14,14,14,14,9,9,14,14,9,14,14,14,12,9,14,14,12,14,14,14,9,9,14,14,14,14,14,14,12,9,14,14,14,14,14,14,9,9,14,14,14,14,14,14,12,9,14,14,14,14,14,14,12,};
static Sprite backMap_bg9 = {0, 8, 8, backMap_bg9_data
};
static unsigned short backMap_bg10_data[] = {
12,12,9,9,9,9,9,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,12,12,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,14,12,12,14,14,14,14,14,14,14,14,14,14,12,12,14,9,9,9,9,9,9,9,12,};
static Sprite backMap_bg10 = {0, 8, 8, backMap_bg10_data
};
static unsigned short backMap_bg11_data[] = {
12,12,9,9,9,9,9,12,9,9,9,9,9,9,14,14,9,9,9,14,14,14,14,14,9,9,14,14,14,14,12,14,9,14,9,14,14,14,14,14,9,14,14,14,14,14,14,14,9,14,14,14,14,14,14,12,9,14,14,14,14,14,9,12,};
static Sprite backMap_bg11 = {0, 8, 8, backMap_bg11_data
};
static unsigned short backMap_bg12_data[] = {
12,12,9,9,9,9,9,12,14,9,9,9,9,9,9,9,14,14,14,14,9,12,12,12,14,14,14,14,14,9,12,9,14,14,14,14,14,14,14,12,14,14,14,14,14,14,14,9,9,14,14,14,14,14,14,12,9,9,14,14,14,14,14,14,};
static Sprite backMap_bg12 = {0, 8, 8, backMap_bg12_data
};
static unsigned short backMap_bg13_data[] = {
12,12,14,14,14,14,14,12,9,14,14,14,14,14,14,9,14,9,14,14,9,14,12,12,14,14,14,12,14,14,12,9,14,14,14,14,14,9,12,12,14,14,14,14,9,9,9,9,9,12,12,12,9,12,12,12,9,9,9,9,9,9,9,12,};
static Sprite backMap_bg13 = {0, 8, 8, backMap_bg13_data
};
static unsigned short backMap_bg14_data[] = {
12,14,14,14,14,14,9,12,9,14,14,14,14,14,14,14,9,9,14,14,14,14,14,14,9,9,14,14,14,14,14,14,9,12,9,14,14,14,12,14,9,12,12,9,14,14,14,14,9,12,12,12,9,12,12,12,9,9,9,9,9,9,9,12,};
static Sprite backMap_bg14 = {0, 8, 8, backMap_bg14_data
};
static unsigned short backMap_bg15_data[] = {
12,12,9,9,9,9,9,12,9,9,9,10,10,9,9,9,9,9,10,10,10,10,12,12,9,9,10,10,10,10,10,9,9,10,10,10,10,10,10,12,9,10,10,15,10,10,10,9,9,12,12,15,15,15,15,12,9,9,9,15,15,15,9,12,};
static Sprite backMap_bg15 = {0, 8, 8, backMap_bg15_data
};
static unsigned short backMap_bg16_data[] = {
12,12,9,15,15,16,9,12,9,9,9,15,15,15,9,9,9,9,9,15,15,16,12,12,9,9,9,15,15,16,12,9,9,12,16,15,16,15,12,12,9,16,15,15,16,15,16,9,9,15,15,15,15,15,15,12,9,9,9,9,9,9,9,12,};
static Sprite backMap_bg16 = {0, 8, 8, backMap_bg16_data
};
static unsigned short backMap_bg17_data[] = {
12,12,9,9,9,9,9,12,9,9,9,9,9,9,9,9,9,9,9,9,9,12,12,12,9,9,9,12,12,9,12,9,9,12,9,6,9,9,12,12,9,12,6,5,6,9,9,9,9,12,12,6,9,12,12,12,9,9,9,9,9,9,9,12,};
static Sprite backMap_bg17 = {0, 8, 8, backMap_bg17_data
};
static unsigned short backMap_map_data[] = {
8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,18,8,8,8,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,17,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,12,11,11,11,13,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,18,8,8,18,9,10,8,8,8,10,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,10,8,8,8,10,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,16,8,8,8,8,10,8,8,8,10,8,18,8,9,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,17,8,8,8,8,10,8,8,8,10,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,18,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,16,8,8,8,8,18,8,10,8,8,8,10,8,8,8,1,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,17,8,8,8,8,8,8,10,8,8,8,10,8,9,8,1,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,10,8,8,8,10,8,8,8,1,8,8,18,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,15,11,11,11,14,8,8,8,1,8,8,8,8,8,8,8,9,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,1,8,8,8,1,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,1,8,8,8,1,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,1,8,8,9,1,8,8,8,1,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,1,8,8,8,1,8,8,1,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,1,8,8,8,1,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,9,8,1,1,8,8,1,8,8,1,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,1,8,8,1,8,8,1,8,9,8,8,8,8,8,8,8,9,8,8,8,8,8,16,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,17,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,9,1,8,8,8,8,8,8,1,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,8,1,1,1,8,8,9,1,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,1,8,1,9,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,1,1,1,1,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,8,0,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,};
static Sprite backMap_map = {0, 128, 60, backMap_map_data
};
Sprite backMap_bgs[] = {
backMap_bg0,
backMap_bg1,
backMap_bg2,
backMap_bg3,
backMap_bg4,
backMap_bg5,
backMap_bg6,
backMap_bg7,
backMap_bg8,
backMap_bg9,
backMap_bg10,
backMap_bg11,
backMap_bg12,
backMap_bg13,
backMap_bg14,
backMap_bg15,
backMap_bg16,
backMap_bg17,
};
int backMap_flags[] = {
0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,0,0,1,};
Background backMap = {backMap_map, backMap_bgs, backMap_flags, 18};
typedef struct{Sprite* sprite; int duration;} AnimKey;
typedef struct{AnimKey* keys; int keyCount;} SpriteAnim;
static unsigned short simple_anim_key0_sprite_data[] = {
0,0,0,1,1,0,0,0,0,0,1,1,1,1,0,0,0,1,1,0,0,1,1,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,};
static Sprite simple_anim_key0_sprite = {0, 8, 8, simple_anim_key0_sprite_data
};
AnimKey simple_anim_key0 = {&simple_anim_key0_sprite, 20};
static unsigned short simple_anim_key1_sprite_data[] = {
0,1,1,1,0,0,0,0,0,1,0,1,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,1,1,1,1,0,0,};
static Sprite simple_anim_key1_sprite = {0, 8, 8, simple_anim_key1_sprite_data
};
AnimKey simple_anim_key1 = {&simple_anim_key1_sprite, 20};
static unsigned short simple_anim_key2_sprite_data[] = {
0,0,1,1,1,1,0,0,0,1,1,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,0,0,0,0,0,0,0,1,1,1,1,0,0,};
static Sprite simple_anim_key2_sprite = {0, 8, 8, simple_anim_key2_sprite_data
};
AnimKey simple_anim_key2 = {&simple_anim_key2_sprite, 20};
static unsigned short simple_anim_key3_sprite_data[] = {
0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,0,0,0,0,};
static Sprite simple_anim_key3_sprite = {0, 8, 8, simple_anim_key3_sprite_data
};
AnimKey simple_anim_key3 = {&simple_anim_key3_sprite, 20};
static unsigned short simple_anim_key4_sprite_data[] = {
0,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,};
static Sprite simple_anim_key4_sprite = {0, 8, 8, simple_anim_key4_sprite_data
};
AnimKey simple_anim_key4 = {&simple_anim_key4_sprite, 20};
static unsigned short simple_anim_key5_sprite_data[] = {
0,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,};
static Sprite simple_anim_key5_sprite = {0, 8, 8, simple_anim_key5_sprite_data
};
AnimKey simple_anim_key5 = {&simple_anim_key5_sprite, 20};
static unsigned short simple_anim_key6_sprite_data[] = {
0,0,1,1,1,1,1,0,0,1,1,0,0,0,1,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,1,1,1,0,0,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,1,1,1,1,0,};
static Sprite simple_anim_key6_sprite = {0, 8, 8, simple_anim_key6_sprite_data
};
AnimKey simple_anim_key6 = {&simple_anim_key6_sprite, 20};
static unsigned short simple_anim_key7_sprite_data[] = {
0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,1,1,1,1,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,};
static Sprite simple_anim_key7_sprite = {0, 8, 8, simple_anim_key7_sprite_data
};
AnimKey simple_anim_key7 = {&simple_anim_key7_sprite, 20};
static unsigned short simple_anim_key8_sprite_data[] = {
0,1,1,1,1,1,1,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,1,1,1,1,1,1,0,};
static Sprite simple_anim_key8_sprite = {0, 8, 8, simple_anim_key8_sprite_data
};
AnimKey simple_anim_key8 = {&simple_anim_key8_sprite, 20};
static unsigned short simple_anim_key9_sprite_data[] = {
0,1,1,1,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,1,0,0,1,1,0,0,0,1,1,1,1,1,0,0,0,0,1,1,1,0,0,0,};
static Sprite simple_anim_key9_sprite = {0, 8, 8, simple_anim_key9_sprite_data
};
AnimKey simple_anim_key9 = {&simple_anim_key9_sprite, 20};
static unsigned short simple_anim_key10_sprite_data[] = {
0,1,0,0,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,1,0,};
static Sprite simple_anim_key10_sprite = {0, 8, 8, simple_anim_key10_sprite_data
};
AnimKey simple_anim_key10 = {&simple_anim_key10_sprite, 20};
static unsigned short simple_anim_key11_sprite_data[] = {
0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,1,1,1,1,0,};
static Sprite simple_anim_key11_sprite = {0, 8, 8, simple_anim_key11_sprite_data
};
AnimKey simple_anim_key11 = {&simple_anim_key11_sprite, 20};
static unsigned short simple_anim_key12_sprite_data[] = {
0,1,0,0,0,0,1,0,0,1,1,0,0,1,1,0,0,1,1,1,1,1,1,0,0,1,0,1,1,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,0,1,0,0,0,0,1,0,};
static Sprite simple_anim_key12_sprite = {0, 8, 8, simple_anim_key12_sprite_data
};
AnimKey simple_anim_key12 = {&simple_anim_key12_sprite, 20};
static unsigned short simple_anim_key13_sprite_data[] = {
0,1,1,0,0,0,1,0,0,1,1,0,0,0,1,0,0,1,1,1,0,0,1,0,0,1,0,1,1,0,1,0,0,1,0,0,1,0,1,0,0,1,0,0,1,1,1,0,0,1,0,0,0,1,1,0,0,1,0,0,0,0,1,0,};
static Sprite simple_anim_key13_sprite = {0, 8, 8, simple_anim_key13_sprite_data
};
AnimKey simple_anim_key13 = {&simple_anim_key13_sprite, 20};
AnimKey simple_anim_keys[] = {
simple_anim_key0,
simple_anim_key1,
simple_anim_key2,
simple_anim_key3,
simple_anim_key4,
simple_anim_key5,
simple_anim_key6,
simple_anim_key7,
simple_anim_key8,
simple_anim_key9,
simple_anim_key10,
simple_anim_key11,
simple_anim_key12,
simple_anim_key13,
};
SpriteAnim simple_anim = { simple_anim_keys, 14 };static unsigned short simple_anim_rev_key0_sprite_data[] = {
0,0,0,0,0,0,0,0,0,9,9,0,0,0,9,9,0,0,9,9,0,9,9,0,0,0,0,9,9,9,0,0,0,0,0,9,9,9,0,0,0,0,9,9,9,9,9,0,0,0,9,9,9,9,9,0,0,0,0,0,0,0,0,0,};
static Sprite simple_anim_rev_key0_sprite = {0, 8, 8, simple_anim_rev_key0_sprite_data
};
AnimKey simple_anim_rev_key0 = {&simple_anim_rev_key0_sprite, 60};
static unsigned short simple_anim_rev_key1_sprite_data[] = {
0,0,0,0,0,0,0,0,0,9,9,0,0,0,9,9,0,9,9,9,0,9,9,9,0,0,0,9,9,9,0,0,0,0,0,9,9,9,0,0,0,0,9,9,9,9,9,0,0,0,9,9,9,9,9,0,0,0,0,0,0,0,0,0,};
static Sprite simple_anim_rev_key1_sprite = {0, 8, 8, simple_anim_rev_key1_sprite_data
};
AnimKey simple_anim_rev_key1 = {&simple_anim_rev_key1_sprite, 50};
static unsigned short simple_anim_rev_key2_sprite_data[] = {
0,0,0,0,0,0,0,0,0,0,9,9,0,9,9,0,0,0,0,9,0,9,0,0,0,0,0,9,9,9,0,0,0,0,0,9,9,9,0,0,0,0,9,9,9,9,9,0,0,0,9,9,9,9,9,0,0,0,0,0,0,0,0,0,};
static Sprite simple_anim_rev_key2_sprite = {0, 8, 8, simple_anim_rev_key2_sprite_data
};
AnimKey simple_anim_rev_key2 = {&simple_anim_rev_key2_sprite, 40};
static unsigned short simple_anim_rev_key3_sprite_data[] = {
0,0,1,1,0,0,0,0,0,1,0,1,1,0,0,0,0,1,0,0,1,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,0,1,0,0,1,1,0,0,0,1,0,1,1,0,0,0,0,1,1,1,0,0,0,0,};
static Sprite simple_anim_rev_key3_sprite = {0, 8, 8, simple_anim_rev_key3_sprite_data
};
AnimKey simple_anim_rev_key3 = {&simple_anim_rev_key3_sprite, 20};
static unsigned short simple_anim_rev_key4_sprite_data[] = {
0,1,1,1,1,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,1,1,1,1,0,0,};
static Sprite simple_anim_rev_key4_sprite = {0, 8, 8, simple_anim_rev_key4_sprite_data
};
AnimKey simple_anim_rev_key4 = {&simple_anim_rev_key4_sprite, 20};
static unsigned short simple_anim_rev_key5_sprite_data[] = {
10,9,9,9,9,9,9,9,9,9,9,10,10,10,9,9,9,9,9,9,9,9,10,10,9,10,9,10,9,9,9,10,9,10,9,9,10,9,9,10,9,9,10,9,9,10,10,9,9,9,10,9,10,9,9,9,9,9,9,10,10,9,9,10,};
static Sprite simple_anim_rev_key5_sprite = {0, 8, 8, simple_anim_rev_key5_sprite_data
};
AnimKey simple_anim_rev_key5 = {&simple_anim_rev_key5_sprite, 20};
static unsigned short simple_anim_rev_key6_sprite_data[] = {
6,6,11,11,11,6,11,11,6,6,6,6,11,6,11,11,11,6,11,6,6,6,6,11,11,6,6,11,6,6,6,6,11,11,6,11,6,11,6,6,6,11,6,11,6,11,6,6,6,6,11,6,11,11,11,6,11,11,11,11,11,6,11,11,};
static Sprite simple_anim_rev_key6_sprite = {0, 8, 8, simple_anim_rev_key6_sprite_data
};
AnimKey simple_anim_rev_key6 = {&simple_anim_rev_key6_sprite, 20};
static unsigned short simple_anim_rev_key7_sprite_data[] = {
3,7,3,7,7,3,7,7,7,7,7,7,7,7,8,7,7,7,8,7,7,3,7,8,7,8,7,8,8,7,8,8,7,8,7,7,8,7,7,7,7,8,8,7,8,7,7,8,7,7,7,8,8,7,7,8,7,3,7,7,7,7,7,7,};
static Sprite simple_anim_rev_key7_sprite = {0, 8, 8, simple_anim_rev_key7_sprite_data
};
AnimKey simple_anim_rev_key7 = {&simple_anim_rev_key7_sprite, 20};
AnimKey simple_anim_rev_keys[] = {
simple_anim_rev_key0,
simple_anim_rev_key1,
simple_anim_rev_key2,
simple_anim_rev_key3,
simple_anim_rev_key4,
simple_anim_rev_key5,
simple_anim_rev_key6,
simple_anim_rev_key7,
};
SpriteAnim simple_anim_rev = { simple_anim_rev_keys, 8 };static unsigned short monster_anim_key0_sprite_data[] = {
0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0,3,3,4,3,3,0,0,0,3,4,4,4,3,0,0,0,3,4,4,4,3,0,0,3,3,3,3,3,3,0,3,3,3,3,3,3,3,3,3,3,0,0,0,0,3,3,};
static Sprite monster_anim_key0_sprite = {0, 8, 8, monster_anim_key0_sprite_data
};
AnimKey monster_anim_key0 = {&monster_anim_key0_sprite, 27};
static unsigned short monster_anim_key1_sprite_data[] = {
0,0,0,0,0,0,0,0,0,0,0,3,3,3,0,0,0,0,3,3,4,3,3,0,0,0,3,4,4,4,3,0,0,0,3,4,4,4,3,0,0,3,3,3,3,3,3,0,0,3,3,3,3,3,3,0,0,3,3,3,0,3,3,0,};
static Sprite monster_anim_key1_sprite = {0, 8, 8, monster_anim_key1_sprite_data
};
AnimKey monster_anim_key1 = {&monster_anim_key1_sprite, 20};
AnimKey monster_anim_keys[] = {
monster_anim_key0,
monster_anim_key1,
};
SpriteAnim monster_anim = { monster_anim_keys, 2 };rgb15 paletteColors[] = {
0,32767,9513,4607,4221,991,30336,1625,543,9924,3990,25895,4775,1855,10743,6512,5356,};
