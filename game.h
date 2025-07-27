#include "common.h"
#include <time.h>

extern int finalLevel;

void game(user *currUser);
void free1(int ***currMap, movingEnemy **currEnemies, pipes **currPipes, predatoryFlower **currPFlowers, coinBlock **currcoinBlocks);
void disableCursorBlinking();
int handleCellCollision(int directionOfCollisio, int celli, int cellj);
int pipe();
void motion();
void gravity();
void jump();
void enemyMotion();
void handleInput();
void printInfo(time_t startTime);
int timer(time_t startTime);
void preparation(int ***currMap, movingEnemy **currEnemies, pipes **currPipes, predatoryFlower **currPFlowers, coinBlock **currcoinBlocks, int currLevel);
int endlevel();
void setConsoleSize(int width, int height);
void fcoinBlock(int celli,int cellj);
void mushroomMotion();
void PFlower();
void actions();

