#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <time.h>
#include <windows.h>
#include <mmsystem.h>

#include "game.h"
#include "common.h"
#include "panel.h"
#include "authmanager.h"


void editInformation(user *currUser)
{
    system("cls");

    int op = 0;

    user search;
    char newStr[50];

    while (1)
    {
        op = 0;
        printf("1. change username \n");
        printf("2. change password \n");
        printf("3. change email \n");
        scanf("%d", &op);

        if (op == 1)
        {
            while (1)
            {
                system("cls");
                printf("Enter your new username : ");
                scanf("%s", newStr);
                int isduplicate = 0;

                // بررسی کردن غیر تکراری بودن نام کاربری
                for (int i = 0;; i++)
                {
                    fseek(f, i * sizeof(search), SEEK_SET);
                    if (fread(&search, sizeof(search), 1, f) == 1)
                    {
                        if (!(strcmp(search.username, newStr)))
                        {
                            isduplicate = 1;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                if (isduplicate)
                {
                    printf("\nThe username is duplicate...");
                    Sleep(1000);
                    continue;
                }

                strcpy(currUser->username, newStr);

                //بروز کردن نام کاربری در فایل
                for (int i = 0;; i++)
                {
                    fseek(f, i * sizeof(search), SEEK_SET);
                    fread(&search, sizeof(search), 1, f);
                    if (search.id == currUser->id)
                    {
                        fseek(f, i * sizeof(search), SEEK_SET);
                        fwrite(currUser, sizeof(search), 1, f);
                        printf("Changed successfully...");
                        Sleep(1000);
                        return;
                    }
                }

            }
        }
        else if (op == 2)
        {
            char rePassword[50];
            while (1)
            {
                system("cls");
                printf("Enter your new password : ");

                // گرفتن رمز عبور با نمایش امن * در کنسول
                int i = 0;
                while (1)
                {

                    newStr[i] = _getch();
                    if (newStr[i] == '\b')
                    {
                        if (i > 0)
                        {
                            printf("\b \b");
                            i--;
                            continue;
                        }
                    }
                    if (newStr[i] == '\r')
                    {
                        newStr[i] = '\0';
                        break;
                    }
                    printf("*");
                    i++;
                }
                if (i < 8)
                {
                    printf("\n Password must be at least eight characters ...");
                    Sleep(1000);
                    continue;
                }
                printf("\nRe-enter your password : ");

                // گرفتن تکرار رمز عبور با نمایش امن * در کنسول
                i = 0;
                while (1)
                {
                    rePassword[i] = _getch();
                    if (rePassword[i] == '\b')
                    {
                        if (i > 0)
                        {
                            printf("\b \b");
                            i--;
                            continue;
                        }
                    }
                    if (rePassword[i] == '\r')
                    {
                        rePassword[i] = '\0';
                        break;
                    }
                    printf("*");
                    i++;
                }
                if (strcmp(rePassword, newStr) != 0)
                {
                    printf("\nPasswords do not match...");
                    Sleep(1000);
                    continue;
                }
                
                //بروز کردن رمز در فایل
                strcpy(currUser->password, newStr);
                for (int i = 0;; i++)
                {
                    fseek(f, i * sizeof(search), SEEK_SET);
                    fread(&search, sizeof(search), 1, f);
                    if (search.id == currUser->id)
                    {
                        fseek(f, i * sizeof(search), SEEK_SET);
                        fwrite(currUser, sizeof(search), 1, f);
                        printf("\nChanged successfully...");
                        Sleep(1000);
                        return;
                    }
                }
            }
        }
        else if (op == 3)
        {
            while (1)
            {
                system("cls");
                printf("Enter your new email : ");
                scanf("%s", newStr);
                strcpy(currUser->email, newStr);
                for (int i = 0;; i++)
                {
                    fseek(f, i * sizeof(search), SEEK_SET);
                    fread(&search, sizeof(search), 1, f);
                    if (search.id == currUser->id)
                    {
                        fseek(f, i * sizeof(search), SEEK_SET);
                        fwrite(currUser, sizeof(search), 1, f);
                        printf("\nChanged successfully...");
                        Sleep(1000);
                        return;
                    }
                }
            }
        }
    }
}

void readHistory(user *currUser)
{
    system("cls");
    FILE *history = fopen("history.bin", "rb");
    if (history != NULL)
        ;
    else
    {
        printf("There is no data to display ...");
        Sleep(1000);
        return;
    }
    record search;
    printf("%-15s %-15s %-15s %-15s %-15s\n", "Level", "Last coin", "Last score", "Losses", "Retries");
    printf("🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱\n");
    fseek(history, 0, SEEK_SET);
    for (int i = 0;; i++)
    {
        if (fread(&search, sizeof(search), 1, history) == 1)
        {
            if (search.userID == currUser->id)
            {
                printf("%-15d %-15d %-15d %-15d %-15d\n", search.mapNum, search.lastCoin, search.lastScore, search.numberOfLosses, search.numberOfRetries);
                if (search.mapNum == (finalLevel + 1))
                {
                    printf("-------------------------------------------------------------------------------\n"); // جهت بخش بندی هر دور بازی که شامل طی کردن 2 مرحله است
                }
            }
        }
        else
        {
            break;
        }
    }
    printf("\npress any key to back ...");
    _getch();

    fclose(history);
}

void panel(user *currUser)
{
    int op = 0;
    while (1)
    {
        op = 0;
        system("cls");
        printf("Hello %s  🖐🏻😃.\n\n", currUser->username);
        printf("🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱🧱\n");
        printf("🚀 1. Edit information 📝\n");
        printf("🚀 2. Game History 📊\n");
        printf("🚀 3. Start a new game 🤖\n");
        printf("🚀 4. exit 🚪\n\n");
        printf("➡  ");
        scanf("%d", &op);
        if (op == 1)
        {
            editInformation(currUser);
        }
        else if (op == 2)
        {
            readHistory(currUser);
        }
        else if (op == 3)
        {
            game(currUser);
        }
        else if (op == 4)
        {
            return;
        }
        else
        {
            printf("\nPlease enter a valid value...\n");
            Sleep(1000);
        }
    }
}