#include <stdio.h>
#include <conio.h>
#include <string.h>
#include <windows.h>
#include "common.h"
#include "authmanager.h"

FILE *f;
void forgotPassword();
int login();
void signUp();
int menu();

void forgotPassword()
{
    user currUser;
    char username[50], password[50], email[50], rePassword[50];

    while (1)
    {
        system("cls");
        printf("Your username : ");
        scanf("%s", username);
        printf("Your email : ");
        scanf("%s", email);

        for (int i = 0;; i++)
        {
            fseek(f, i * sizeof(currUser), SEEK_SET);
            if (fread(&currUser, sizeof(currUser), 1, f) == 1)
            {
                if (!(strcmp(currUser.username, username) || strcmp(currUser.email, email)))
                {
                    while (1)
                    {
                        printf("Please enter your new password :");
                        int i = 0;
                        while (1)
                        {
                            password[i] = _getch();
                            if (password[i] == '\b')
                            {
                                if (i > 0)
                                {
                                    printf("\b \b");
                                    i--;
                                    continue;
                                }
                            }
                            if (password[i] == '\r')
                            {
                                password[i] = '\0';
                                break;
                            }
                            printf("*");
                            i++;
                        }

                        if (i < 8)
                        {
                            printf("\n Password must be at least eight characters ...");
                            continue;
                        }

                        printf("\nRe-enter your password : ");

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

                        if (strcmp(rePassword, password) != 0)
                        {
                            printf("\nPasswords do not match...");
                            Sleep(1000);
                            continue;
                        }
                        strcpy(currUser.password, password);
                        fseek(f, i * sizeof(currUser), SEEK_SET);
                        fwrite(&currUser, sizeof(currUser), 1, f);
                        printf("\n Password changed successfully...");
                        Sleep(1000);
                        return;
                    }
                }
            }
            else
            {
                break;
            }
        }
        printf("There is no user with these characteristics ...");
        Sleep(1000);
        continue;
    }
}

int login()
{
    user currUser;
    char username[50], password[50], op[3];

    while (1)
    {
        printf("Your username : ");
        scanf("%s", username);
        printf("Your password : ");

        int i = 0;
        while (1)
        {

            password[i] = _getch();
            if (password[i] == '\b')
            {
                if (i > 0)
                {
                    printf("\b \b");
                    i--;
                    continue;
                }
            }
            if (password[i] == '\r')
            {
                password[i] = '\0';
                break;
            }
            printf("*");
            i++;
        }

        for (int i = 0;; i++)
        {
            fseek(f, i * sizeof(currUser), SEEK_SET);
            if (fread(&currUser, sizeof(currUser), 1, f) == 1)
            {
                if (!(strcmp(currUser.username, username) || strcmp(currUser.password, password)))
                {
                    return currUser.id;
                }
            }
            else
            {
                break;
            }
        }
        printf("\nThe username or password is incorrect.\n");
        printf("Have you forgotten your password?(Yes/No) : ");
        scanf("%s", op);
        if (!strcmp("Yes", op))
            forgotPassword();
        system("cls");
    }
}

void signUp()
{
    user currUser;
    char username[50], password[50], rePassword[50], email[50];

    while (1)
    {
        system("cls");
        printf("Enter your desired username : ");
        scanf("%s", username);
        printf("Enter your email : ");
        scanf("%s", email);
        printf("Enter your desired password (at least 8 characters): ");

        int i = 0;
        while (1)
        {
            password[i] = _getch();
            if (password[i] == '\b')
            {
                if (i > 0)
                {
                    printf("\b \b");
                    i--;
                    continue;
                }
            }
            if (password[i] == '\r')
            {
                password[i] = '\0';
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

        if (strcmp(rePassword, password) != 0)
        {
            printf("\nPasswords do not match...");
            Sleep(1000);
            continue;
        }
        int isduplicate = 0;
        for (int i = 0;; i++)
        {
            fseek(f, i * sizeof(currUser), SEEK_SET);
            if (fread(&currUser, sizeof(currUser), 1, f) == 1)
            {
                if (!(strcmp(currUser.username, username)))
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
        user newUser;
        strcpy(newUser.username, username);
        strcpy(newUser.password, password);
        strcpy(newUser.email, email);
        fseek(f, 0, SEEK_END);
        int id = ftell(f) / sizeof(currUser);
        id++;
        newUser.id = id;
        fwrite(&newUser, sizeof(currUser), 1, f);

        printf("\nRegistration Successful\nRedirecting ...");
        Sleep(1000);
        system("cls");
        break;
    }
}

int menu()
{
    f = fopen("users.bin", "r+b");
    if (f)
        ;
    else
        f = fopen("users.bin", "w+b");

    while (1)
    {
        int op = 0;
        printf("Please enter your desired option.\n");
        printf("-----------------------------------------\n\n");
        printf("1.login\n");
        printf("2.signUp\n");
        scanf("%d", &op);
        getchar();
        if (op == 1)
        {
            system("cls");
            return login();
        }
        else if (op == 2)
        {
            system("cls");
            signUp();
        }
        else
        {
            printf("Please enter a valid value...\n");
        }
        Sleep(1000);
        system("cls");
    }
}
