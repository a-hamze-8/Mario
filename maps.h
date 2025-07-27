#ifndef MAPS
#define MAPS

#include "common.h"

#define NUMBERSOFMAPS 2
extern int **level1;
extern int **level2;
extern int elementOfMaps[NUMBERSOFMAPS][4];
extern int marioPos[NUMBERSOFMAPS][2];
extern int dimensions[NUMBERSOFMAPS][2];
extern emoji emojis[19];

void disply(int **currMap,int row, int col);
void loadlevels();
void loadMap(int ** currMap,movingEnemy * currEnemies,pipes *currPipes,predatoryFlower *currPFlowers,state *mario,coinBlock *currcoinBlocks,int currLevel,int mushroom[]);
void updateMEnemey(movingEnemy *currEnemies, int currLevel);
void free2();

#endif