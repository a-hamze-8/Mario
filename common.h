#ifndef COMMON
#define COMMON

typedef struct
{
    int i;
    int j;
    int dir;
    int isJumping;
    int lives;
    int haveShield;
    int isBig;
    int isCrouching;

}state;

typedef struct
{
    int origin;
    int disI;
    int disJ;
    int dir;
} pipes;

typedef struct
{
    int i;
    int j;
    int dir;
    int entity;
} movingEnemy;

typedef struct
{
    int i;
    int j;
    int display;
} predatoryFlower;

typedef struct
{
    int i;
    int j;
    int NumOfCoins;
} coinBlock;

typedef struct
{
    char *uCode;
    int i;
} emoji;

typedef struct
{
    char username[50];
    char password[50];
    char email[50];
    int id;
} user;

typedef struct
{
    int userID;
    int lastCoin;
    int lastScore;
    int numberOfLosses;
    int numberOfRetries;
    int mapNum;
} record;

#endif