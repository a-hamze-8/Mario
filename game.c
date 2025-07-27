#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <mmsystem.h>
#include "helper_windows.h"
#include "maps.h"
#include "common.h"
#include "game.h"
#include "authmanager.h"

// متغییر های لازم جهت چاپ ماریو
char marioCh[] = "🤖";
long unsigned int bw;

// نگهدارنده اطلاعات این دور بازی کاربر
record currRecord[NUMBERSOFMAPS];

//شماره مرحله فعلی کاربر
int currLevel = 0;

//شماره مرحله اخر
int finalLevel = NUMBERSOFMAPS - 1;

// نگهدارنده مپ فعلی بازی که در حین بازی تغییرات را حفظ میکند
int **currMap;

//نگهدارند ویژگی های عناصر موثر بازی
movingEnemy *currEnemies;
pipes *currPipes;
predatoryFlower *currPFlowers;
coinBlock *currcoinBlocks;

// نگهدارنده ویژگی ها ماریو (موقعیت ، وضعیت حرکت ، بزرگ یا کوچک بودن ، نشسته بودن)
state mario;

// بررسی ادامه داشتن هر دست بازی (به هر تلاش ماریو جهت رسیدن پرچم یک دست بازی گفته میشود)
int checkGameStatus = 1;

// بررسی رسیدن به پرچم
int checkendlevel = 0;

//نگه دارنده زمان شروع هر دست
time_t startTime;

// جهت قانون افزایش امتیاز در نابودی دشمن در زیر 5 ثانیه
time_t lastRemove = 0;

// نگه دارنده موقعیت و وضعیت قارچ
int mushroom[4] = {0, 0, 0, 1};

//میزان امتیاز برای حذف دشمن عادی
int scoreOfRemove = 100;

//غیر فعال کردن نمایش کرسر
void disableCursorBlinking()
{
    CONSOLE_CURSOR_INFO cursorInfo;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

// تنظیم اندازه کنسول
void setConsoleSize(int width, int height)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    COORD bufferSize;
    bufferSize.X = width;
    bufferSize.Y = height;
    SetConsoleScreenBufferSize(hConsole, bufferSize);

    SMALL_RECT windowSize;
    windowSize.Left = 0;
    windowSize.Top = 0;
    windowSize.Right = width - 1;
    windowSize.Bottom = height - 1;
    SetConsoleWindowInfo(hConsole, TRUE, &windowSize);
}

void free1(int ***currMap, movingEnemy **currEnemies, pipes **currPipes, predatoryFlower **currPFlowers, coinBlock **currcoinBlocks)
{
    for (int i = 0; i < dimensions[currLevel][0]; i++)
    {
        free(*(*(currMap) + i));
    }
    free(*currMap);
    free(*currEnemies);
    free(*currPipes);
    free(*currPFlowers);
    free(*currcoinBlocks); 
}

//انجام عملیات های مپ بازی در حین حرکت ماریو
void actions()
{
    int counter = 0;
    while (mario.lives > 0 && timer(startTime) > 0)
    {
        if (counter % 16 == 0)
        {
            enemyMotion();
        }
        if (counter % 96 == 0)
        {
            PFlower();
            counter = 1;
        }
        if (mushroom[0] && counter % 8 == 0)
        {
            mushroomMotion();
        }
        Sleep(20);
        counter++;
    }
    return;
}


void game(user *currUser)
{
    disableCursorBlinking();

    // بارگذاری تمامی نقشه ها و المان های موثر
    loadlevels();

    // صفر کردن رکورد ها
    for (int i = 0; i < NUMBERSOFMAPS; i++)
    {
        currRecord[i].userID = currUser->id;
        currRecord[i].lastCoin = 0;
        currRecord[i].lastScore = 0;
        currRecord[i].mapNum = 0;
        currRecord[i].numberOfLosses = 0;
        currRecord[i].numberOfRetries = 0;
    }

    currLevel = 0;
    while (1)
    {
        
        setConsoleSize(dimensions[currLevel][1] * 2 + 2, dimensions[currLevel][0] + 2);

        // تخصیص حافظه به ارایه های پویا (به دلیل متغییر بودن تعداد هر کدام در هر یک از مپ ها)
        preparation(&currMap, &currEnemies, &currPipes, &currPFlowers, &currcoinBlocks, currLevel);

        // ریست تعداد جان های ماریو
        mario.lives = 3;

        currRecord[currLevel].lastCoin = 0;
        currRecord[currLevel].lastScore = 0;
        currRecord[currLevel].mapNum = currLevel + 1;

        checkGameStatus = 1;

        //بروز کردن تعداد و موقعیت دشمنان متحرک
        updateMEnemey(currEnemies, currLevel);

        // اجرای هم زمان گرفتن ورودی و انجام فعالیت های مربوط به مپ
        HANDLE thread1 = start_listening(handleInput);
        HANDLE thread2 = start_listening(actions);

        startTime = time(NULL);

        while (mario.lives > 0 && timer(startTime) > 0 && !checkendlevel)
        {
            // بارگذاری و نمایش مرحله ای که در ان هستیم
            loadMap(currMap, currEnemies, currPipes, currPFlowers, &mario, currcoinBlocks, currLevel, mushroom);
            disply(currMap, dimensions[currLevel][0], dimensions[currLevel][1]);

            checkGameStatus = 1;

            //اجرای حرکت ماریو
            while (checkGameStatus && timer(startTime) > 0)
            {

                if (mario.isJumping == 1)
                {
                    PlaySound("sounds\\smb_jump-super.wav", NULL, SND_FILENAME | SND_ASYNC);
                    jump();
                    mario.isJumping = 0;
                }
                else
                {
                    if (mario.dir != 0)
                    {
                        motion();
                        mario.dir = 0;
                    }
                }
                gravity();

                // نمایش اطلاعات در بالای صفحه
                printInfo(startTime);
                Sleep(20);
            }
        }
        if (_kbhit())
            _getch();

        free1(&currMap, &currEnemies, &currPipes, &currPFlowers, &currcoinBlocks);
        system("cls");
        
        // پایان هر مرحله
        if (checkendlevel)
        {
            
            checkendlevel = 0;

            //پایان تمامی مراحل
            if (endlevel())
            {
                free2();
                return;
            }
        }
        else
        {

            printf("You lost.\n");
            printf("press any key to retray ...");
            currRecord[currLevel].numberOfLosses++;
            _getch();
        }
    }
    free2();
}

void fcoinBlock(int celli, int cellj)
{
    for (int i = 0; i < elementOfMaps[currLevel][3]; i++)
    {
        if (currcoinBlocks[i].i == celli && currcoinBlocks[i].j == cellj)
        {
            if (currcoinBlocks[i].NumOfCoins > 0)
            {
                currcoinBlocks[i].NumOfCoins--;
                PlaySound("sounds\\smb_coin.wav", NULL, SND_FILENAME | SND_ASYNC);
                currRecord[currLevel].lastCoin++;
                break;
            }
            if (currcoinBlocks[i].NumOfCoins == 0)
            {
                char black[] = "🟫";
                COORD pos;
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                pos.X = 2 * (currcoinBlocks[i].j);
                pos.Y = currcoinBlocks[i].i;
                WriteConsoleOutputCharacterA(hConsole, black, strlen(black), pos, &bw);
            }
        }
    }
}

void mushroomMotion()
{
    COORD pos;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    char mush[] = "🍄";
    long unsigned int bw1;
    currMap[mushroom[1]][mushroom[2]] = 16;
    pos.X = 2 * (mushroom[2]);
    pos.Y = mushroom[1];
    WriteConsoleOutputCharacterA(hConsole, mush, strlen(mush), pos, &bw);
    if (currMap[mushroom[1]][mushroom[2] + mushroom[3]] == -1 || currMap[mushroom[1] - 1][mushroom[2]] == -1)
    {
        pos.X = 2 * (mushroom[2]);
        pos.Y = mushroom[1];
        WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        currMap[mushroom[1]][mushroom[2]] = 0;
        mario.isBig = 1;
        mushroom[0] = 0;
        mushroom[1] = 0;
        mushroom[2] = 0;
        mushroom[3] = 1;
        pos.X = mario.j * 2;
        pos.Y = mario.i - 1;
        WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
        currMap[mario.i - 1][mario.j] = -1;
        return;
    }
    if (currMap[mushroom[1]][mushroom[2] + mushroom[3]] != 0)
    {
        mushroom[3] *= -1;
    }
    pos.X = 2 * (mushroom[2]);
    pos.Y = mushroom[1];
    WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
    currMap[mushroom[1]][mushroom[2]] = 0;
    pos.X += mushroom[3] * 2;
    mushroom[2] += mushroom[3];
    WriteConsoleOutputCharacterA(hConsole, mush, strlen(mush), pos, &bw);
    currMap[mushroom[1]][mushroom[2]] = 16;

    while (currMap[mushroom[1] + 1][mushroom[2]] == 0)
    {
        pos.X = 2 * (mushroom[2]);
        pos.Y = mushroom[1];
        WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        currMap[mushroom[1]][mushroom[2]] = 0;
        pos.Y += 1;
        mushroom[1] += 1;
        WriteConsoleOutputCharacterA(hConsole, mush, strlen(mush), pos, &bw);
        currMap[mushroom[1]][mushroom[2]] = 16;
    }
    return;
}

int handleCellCollision(int directionOfCollisio, int celli, int cellj)
{
    COORD pos;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    srand(time(0));
    switch (currMap[celli][cellj])
    {
    case 0:
        return 0;

    case 1:
        return 1;

    case 4:
        PlaySound("sounds\\smb_coin.wav", NULL, SND_FILENAME | SND_ASYNC);
        currRecord[currLevel].lastCoin++;
        return 0;

    case 5:
        if (mario.haveShield)
        {
            for (int i = 0; i < elementOfMaps[currLevel][2]; i++)
            {
                if (currPFlowers[i].i == celli && currPFlowers[i].j == cellj)
                {
                    currPFlowers[i].display = 0;
                    mario.haveShield = 0;
                    return 0;
                }
            }
        }
        if (mario.isBig)
        {
            for (int i = 0; i < elementOfMaps[currLevel][2]; i++)
            {
                if (currPFlowers[i].i == celli && currPFlowers[i].j == cellj)
                {
                    currPFlowers[i].display = 0;
                    break;
                }
            }
            currMap[mario.i - 1][mario.j] = 0;
            pos.X = mario.j * 2;
            pos.Y = mario.i - 1;
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            mario.isBig = 0;
            return 0;
        }
        mario.lives--;
        checkGameStatus = 0;
        return 1;
    case 11:
        if (mario.haveShield)
        {
            mario.haveShield = 0;
            return 0;
        }
        if (mario.isBig)
        {
            currMap[mario.i - 1][mario.j] = 0;
            pos.X = mario.j * 2;
            pos.Y = mario.i - 1;
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            mario.isBig = 0;
            return 0;
        }
        mario.lives--;
        checkGameStatus = 0;
        return 1;

    case 6:
        if (directionOfCollisio == -2)
        {
            if ((int)(time(NULL) - lastRemove) <= 5)
            {
                scoreOfRemove *= 2;
            }
            else
            {
                scoreOfRemove = 100;
            }
            PlaySound("sounds\\mb_touch.wav", NULL, SND_FILENAME | SND_ASYNC);
            currRecord[currLevel].lastScore += scoreOfRemove;
            for (int i = 0; i < elementOfMaps[currLevel][0]; i++)
            {
                if (currEnemies[i].i == celli && currEnemies[i].j == cellj)
                {
                    currEnemies[i].entity = 0;
                    currMap[celli][cellj] = 0;
                    pos.Y = currEnemies[i].i;
                    pos.X = currEnemies[i].j * 2;
                    WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
                    break;
                }
            }
            lastRemove = time(NULL);
            return 0;
        }
        else
        {
            if (mario.haveShield)
            {
                for (int i = 0; i < elementOfMaps[currLevel][0]; i++)
                {
                    if (currEnemies[i].i == celli && currEnemies[i].j == cellj)
                    {
                        currEnemies[i].entity = 0;
                        break;
                    }
                }
                mario.haveShield = 0;
                return 0;
            }
            else if (mario.isBig)
            {
                for (int i = 0; i < elementOfMaps[currLevel][0]; i++)
                {
                    if (currEnemies[i].i == celli && currEnemies[i].j == cellj)
                    {
                        currMap[mario.i - 1][mario.j] = 0;
                        pos.X = mario.j * 2;
                        pos.Y = mario.i - 1;
                        WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
                        currEnemies[i].entity = 0;
                        break;
                    }
                }
                mario.isBig = 0;
                return 0;
            }
            mario.lives--;
            checkGameStatus = 0;
            return 1;
        }

    case 7:
    case 8:
        return 1;

    case 3:
        if (directionOfCollisio == 2)
        {
            fcoinBlock(celli, cellj);
            return 1;
        }
        return 1;

    case 9:
        checkGameStatus = 0;
        currRecord[currLevel].lastScore += (timer(startTime)/10) * 100;
        currRecord[currLevel].lastScore += mario.lives * 1000;
        mario.lives = 0;
        checkendlevel = 1;
        return 0;
    case 2:
        if (mario.isBig && directionOfCollisio == 2)
        {
            currMap[celli][cellj] = 0;
            pos.X = cellj * 2;
            pos.Y = celli;
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            if ((rand() % 10) < 2)
            {
                PlaySound("sounds\\smb_coin.wav", NULL, SND_FILENAME | SND_ASYNC);
                currRecord[currLevel].lastCoin++;
            }
            return 1;
        }
        return 1;
    case 15:
        mario.haveShield = 1;
        return 0;
    case 13:
        if (mushroom[0] == 1)
        {
            return 1;
        }
        if (directionOfCollisio == 2 && !mario.isBig)
        {
            mushroom[0] = 1;
            mushroom[1] = celli - 1;
            mushroom[2] = cellj;
            srand(time(NULL));
            mushroom[3] = (rand() % 2) ? 1 : -1;
            return 1;
        }
        return 1;
    case 16:

        mushroom[0] = 0;
        mushroom[1] = 0;
        mushroom[2] = 0;
        mushroom[3] = 1;
        mario.isBig = 1;
        return 0;
    default:
        return 1;
    }
}

int endlevel()
{
    int op = -1;
    printf("your score : %d 🎈\n", currRecord[currLevel].lastScore);
    printf("your coin : %d 💰\n\n", currRecord[currLevel].lastCoin);
    if (currLevel < finalLevel)
    {
        printf("🚀 1. Retray\n");
        printf("🚀 2. Continue\n\n");
        printf("➡  ");
        scanf("%d", &op);
        if (op == 1)
        {
            currRecord[currLevel].numberOfRetries++;
            return 0;
        }
        else
        {
            currLevel++;
            return 0;
        }
    }
    else
    {
        printf("🚀 1. Retray\n");
        printf("🚀 2. Submit Score\n\n");
        printf("➡  ");
        scanf("%d", &op);
        if (op == 1)
        {
            currRecord[currLevel].numberOfRetries++;
            return 0;
        }
        else
        {
            FILE *history = fopen("history.bin", "r+b");
            if (history == NULL)
            {
                history = fopen("history.bin", "w+b");
            }
            fseek(history, 0, SEEK_END);
            for (int i = 0; i < NUMBERSOFMAPS; i++)
            {
                fwrite(&currRecord[i], sizeof(currRecord[i]), 1, history);
            }

            fclose(history);
            return 1;
        }
    }
}

int pipe()
{
    for (int i = 0; i < elementOfMaps[currLevel][1]; i++)
    {
        if (mario.isBig)
        {
            if ((mario.i - 1) == dimensions[currLevel][0] - 1 || mario.j == dimensions[currLevel][1] - 1 || mario.i == 0 || mario.j == 0)
            {
                return 0;
            }
        }
        else if (mario.i == dimensions[currLevel][0] - 1 || mario.j == dimensions[currLevel][1] - 1 || mario.i == 0 || mario.j == 0)
        {
            return 0;
        }
        if (mario.isBig)
        {
            if (currMap[mario.i][mario.j + 1] == currPipes[i].origin || currMap[mario.i][mario.j - 1] == currPipes[i].origin || currMap[mario.i - 2][mario.j] == currPipes[i].origin || currMap[mario.i + 1][mario.j] == currPipes[i].origin)
            {
                COORD pos;
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                if (currPipes[i].dir == -2)
                {
                    currMap[mario.i][mario.j] = 0;
                    currMap[mario.i - 1][mario.j] = 0;
                    mario.i = currPipes[i].disI + 1;
                    mario.j = currPipes[i].disJ;
                    pos.X = 2 * (mario.j);
                    pos.Y = mario.i;
                    currMap[mario.i][mario.j] = -1;
                    currMap[mario.i - 1][mario.j] = -1;
                    WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
                    pos.Y -= 1;
                    WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
                    return 1;
                }
                currMap[mario.i][mario.j] = 0;
                currMap[mario.i - 1][mario.j] = 0;
                mario.i = currPipes[i].disI;
                mario.j = currPipes[i].disJ;
                pos.X = 2 * (mario.j);
                pos.Y = mario.i;
                currMap[mario.i][mario.j] = -1;
                currMap[mario.i - 1][mario.j] = -1;
                WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
                pos.Y -= 1;
                WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
                return 1;
            }
        }
        if (currMap[mario.i][mario.j + 1] == currPipes[i].origin || currMap[mario.i][mario.j - 1] == currPipes[i].origin || currMap[mario.i - 1][mario.j] == currPipes[i].origin || currMap[mario.i + 1][mario.j] == currPipes[i].origin)
        {
            currMap[mario.i][mario.j] = 0;
            mario.i = currPipes[i].disI;
            mario.j = currPipes[i].disJ;
            COORD pos;
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            pos.X = 2 * (mario.j);
            pos.Y = mario.i;
            currMap[mario.i][mario.j] = -1;
            WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
            return 1;
        }
    }
    return 0;
}

void motion()
{
    if (mario.isCrouching)
    {
        return;
    }
    if (mario.j + mario.dir > dimensions[currLevel][1] - 1)
    {
        checkGameStatus = 0;
        mario.lives--;
        return;
    }
    if (handleCellCollision(mario.dir, mario.i, mario.j + mario.dir))
    {
        return;
    }
    COORD pos;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    if (mario.isBig)
    {
        if (handleCellCollision(mario.dir, mario.i - 1, mario.j + mario.dir))
        {
            return;
        }
        pos.X = 2 * (mario.j);
        pos.Y = mario.i;
        WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        pos.Y = mario.i - 1;
        WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        pos.X += mario.dir * 2;
        WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
        pos.Y = mario.i;
        WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
        currMap[mario.i][mario.j] = 0;
        currMap[mario.i - 1][mario.j] = 0;
        mario.j += mario.dir;
        currMap[mario.i][mario.j] = -1;
        currMap[mario.i - 1][mario.j] = -1;
    }
    else
    {
        pos.X = 2 * (mario.j);
        pos.Y = mario.i;
        WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        pos.X += mario.dir * 2;
        WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
        currMap[mario.i][mario.j] = 0;
        mario.j += mario.dir;
        currMap[mario.i][mario.j] = -1;
    }

    if (pipe())
    {
        if (mario.isBig)
        {
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            pos.Y -= 1;
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        }
        else
        {
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        }
    }
}

void gravity()
{
    COORD pos;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    pos.Y = mario.i;
    while (TRUE)
    {
        if (mario.dir != 0)
            motion();
        if (!checkGameStatus)
        {
            return;
        }
        mario.dir = 0;
        pos.X = 2 * (mario.j);
        pos.Y = mario.i;
        if (mario.i == dimensions[currLevel][0] - 1)
        {
            checkGameStatus = 0;
            mario.lives--;
            return;
        }

        if (handleCellCollision(-2, mario.i + 1, mario.j))
        {
            return;
        }
        if (mario.isBig && !mario.isCrouching)
        {
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            pos.Y -= 1;
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            pos.Y += 1;
            WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
            pos.Y += 1;
            WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);

            currMap[mario.i][mario.j] = 0;
            mario.i -= 1;
            currMap[mario.i][mario.j] = 0;
            mario.i += 1;
            currMap[mario.i][mario.j] = -1;
            mario.i += 1;
            currMap[mario.i][mario.j] = -1;
            Sleep(90);
        }
        else
        {
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            pos.Y += 1;
            WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
            currMap[mario.i][mario.j] = 0;
            mario.i += 1;
            currMap[mario.i][mario.j] = -1;
            Sleep(90);
        }
        if (pipe())
        {
            if (mario.isBig)
            {
                WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
                pos.Y -= 1;
                WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            }
            else
            {
                WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            }
        }
    }
}

void jump()
{
    COORD pos;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    pos.Y = mario.i;

    for (int i = 1; i <= 5; i++)
    {
        if (mario.i - 1 < 0)
        {
            checkGameStatus = 0;
            mario.lives--;
            return;
        }
        if (mario.isBig)
        {
            if (handleCellCollision(2, mario.i - 2, mario.j))
            {
                return;
            }
        }
        else if (handleCellCollision(2, mario.i - 1, mario.j))
        {
            return;
        }

        if (mario.dir != 0)
            motion();
        mario.dir = 0;
        if (!checkGameStatus)
        {
            return;
        }
        pos.X = 2 * (mario.j);
        pos.Y = mario.i;
        if (mario.isBig && !mario.isCrouching)
        {
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            pos.Y -= 1;
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
            pos.Y -= 1;
            WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
            pos.Y += 1;

            currMap[mario.i][mario.j] = 0;
            mario.i -= 1;
            currMap[mario.i][mario.j] = 0;
            currMap[mario.i][mario.j] = -1;
            mario.i -= 1;
            currMap[mario.i][mario.j] = -1;
            mario.i += 1;
            Sleep(90);
        }
        else
        {
            if (currMap[(mario.i) - 1][mario.j] != 0 && checkGameStatus)
                break;
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            pos.Y = mario.i - 1;
            WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
            currMap[mario.i][mario.j] = 0;
            mario.i -= 1;
            currMap[mario.i][mario.j] = -1;
            Sleep(90);
        }
        if (pipe())
        {
            if (mario.isBig)
            {
                WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
                pos.Y -= 1;
                WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            }
            else
            {
                WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
            }
        }
    }
}

void enemyMotion()
{
    COORD pos;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    for (int i = 0; i < elementOfMaps[currLevel][0]; i++)
    {
        if (!currEnemies[i].entity)
        {
            continue;
        }
        char enemy[] = "🦀";
        long unsigned int bw1;
        if (currMap[currEnemies[i].i][currEnemies[i].j + currEnemies[i].dir] == -1)
        {
            if (mario.isBig)
            {
                currMap[mario.i - 1][mario.j] = 0;
                pos.X = mario.j * 2;
                pos.Y = mario.i - 1;
                WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
                currEnemies[i].entity = 0;
                currMap[currEnemies[i].i][currEnemies[i].j] = 0;
                pos.Y = currEnemies[i].i;
                pos.X = currEnemies[i].j * 2;
                WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
                mario.isBig = 0;
                continue;
            }
            else if (mario.haveShield)
            {
                currEnemies[i].entity = 0;
                mario.haveShield = 0;
                continue;
            }
            mario.lives--;
            checkGameStatus = 0;
            return;
        }
        else if (currMap[currEnemies[i].i][currEnemies[i].j + currEnemies[i].dir] != 0)
        {
            currEnemies[i].dir *= -1;
        }
        pos.X = 2 * (currEnemies[i].j);
        pos.Y = currEnemies[i].i;
        WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        pos.X += currEnemies[i].dir * 2;
        WriteConsoleOutputCharacterA(hConsole, enemy, strlen(enemy), pos, &bw);
        if (currMap[currEnemies[i].i][currEnemies[i].j + currEnemies[i].dir] == -1)
        {
            mario.lives--;
            checkGameStatus = 0;
            return;
        }
        currMap[currEnemies[i].i][currEnemies[i].j] = 0;
        currEnemies[i].j += currEnemies[i].dir;
        currMap[currEnemies[i].i][currEnemies[i].j] = 6;
    }
}

void PFlower()
{
    COORD pos;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    char enemy[] = "🌱";
    long unsigned int bw1;
    for (int i = 0; i < elementOfMaps[currLevel][2]; i++)
    {
        currPFlowers[i].display *= -1;
        if (currPFlowers[i].display == 1)
        {
            if (currMap[currPFlowers[i].i][currPFlowers[i].j] == -1)
            {
                if (mario.isBig)
                {
                    currMap[mario.i - 1][mario.j] = 0;
                    currMap[currPFlowers[i].i][currPFlowers[i].j] = 0;
                    pos.X = mario.j * 2;
                    pos.Y = mario.i - 1;
                    WriteConsoleOutputCharacterA(hConsole, " ", 2, pos, &bw);
                    currPFlowers[i].display = 0;
                    mario.isBig = 0;
                    continue;
                }
                else if (mario.haveShield)
                {
                    currPFlowers[i].display = 0;
                    mario.haveShield = 0;
                    continue;
                }
                mario.lives--;
                checkGameStatus = 0;
                return;
            }
            currMap[currPFlowers[i].i][currPFlowers[i].j] = 5;
            pos.Y = currPFlowers[i].i;
            pos.X = currPFlowers[i].j * 2;
            WriteConsoleOutputCharacterA(hConsole, enemy, strlen(enemy), pos, &bw);
        }
        else if (currPFlowers[i].display == -1)
        {
            currMap[currPFlowers[i].i][currPFlowers[i].j] = 0;
            pos.Y = currPFlowers[i].i;
            pos.X = currPFlowers[i].j * 2;
            WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
        }
    }
}

void handleInput()
{
    while (mario.lives > 0 && timer(startTime) > 0)
    {
        if (_kbhit())
        {
            char ch = _getch();
            switch (ch)
            {
            case ' ':
            case 'w':
                if (mario.isJumping == 0)
                    mario.isJumping = 1;
                break;
            case 'a':
                mario.dir = -1;
                break;
            case 'd':
                mario.dir = +1;
                break;
            case 's':
                if (mario.isBig)
                {
                    mario.isCrouching = 1;
                    currMap[mario.i - 1][mario.j] = 0;
                    COORD pos = {mario.j * 2, mario.i - 1};
                    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                    WriteConsoleOutputCharacterA(hConsole, "  ", 2, pos, &bw);
                }
            default:
                break;
            }
        }
        if (!(GetAsyncKeyState('S') & 0x8000) && mario.isCrouching)
        {
            mario.isCrouching = 0;
            if (mario.isBig)
            {
                currMap[mario.i - 1][mario.j] = -1;
                COORD pos = {mario.j * 2, mario.i - 1};
                HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
                WriteConsoleOutputCharacterA(hConsole, marioCh, strlen(marioCh), pos, &bw);
            }
        }
    }

    return;
}

void printInfo(time_t startTime)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    char info[95];
    sprintf(info, "time : %-3d --- coin : %-3d --- score : %-3d --- lives : %-3d %-23s", timer(startTime), currRecord[currLevel].lastCoin, currRecord[currLevel].lastScore, mario.lives, ((mario.haveShield) ? "You have Shield" : "You don't have Shield"));
    short x = (dimensions[currLevel][1] * 2 - strlen(info)) / 2;
    COORD pos = {x, 0};
    long unsigned int bw;
    WriteConsoleOutputCharacterA(hConsole, info, strlen(info), pos, &bw);
}

int timer(time_t startTime)
{
    int elapsed = (int)time(NULL) - startTime;
    return 180 - elapsed;
}

void preparation(int ***currMap, movingEnemy **currEnemies, pipes **currPipes, predatoryFlower **currPFlowers, coinBlock **currcoinBlocks, int currLevel)
{
    *currMap = (int **)malloc(dimensions[currLevel][0] * sizeof(int *));
    for (int i = 0; i < dimensions[currLevel][0]; i++)
    {
        *((*currMap) + i) = (int *)malloc(dimensions[currLevel][1] * sizeof(int));
    }
    *currEnemies = (movingEnemy *)malloc(elementOfMaps[currLevel][0] * sizeof(movingEnemy));
    *currPipes = (pipes *)malloc(elementOfMaps[currLevel][1] * sizeof(pipes));
    *currPFlowers = (predatoryFlower *)malloc(elementOfMaps[currLevel][2] * sizeof(predatoryFlower));
    *currcoinBlocks = (coinBlock *)malloc(elementOfMaps[currLevel][3] * sizeof(coinBlock));
}
