#include <stdio.h>
#include <windows.h>
#include "common.h"
#include "maps.h"
#include "authmanager.h"
#include "panel.h"

int main()
{
    SetConsoleOutputCP(CP_UTF8);  //جهت نمایش ایموجی در کنسول

    int currUserID = -1;
    currUserID = menu(); // صفحه ورود  و ثبت نام
    user currUser;

    // پیدا کردن ID کاربر مربوطه
    for (int i = 0;; i++)
    {
        fseek(f, i * sizeof(currUser), SEEK_SET);
        fread(&currUser, sizeof(currUser), 1, f);
        if (currUser.id == currUserID)
        {
            break;
        }
    }
    
    // انتقال به پنل کاربری
    panel(&currUser);
    fclose(f);
}