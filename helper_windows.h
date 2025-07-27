#ifndef HELPER_WINDOWS_H
#define HELPER_WINDOWS_H


#include <conio.h>
#include <windows.h>
#include <stdio.h>


DWORD WINAPI keypress_callback(LPVOID args) {
	void (*callback)(void) = (void (*)(void))((void *)(args));

	callback();

	return 0;
}

HANDLE start_listening(void (*callback)(void)) {
	HANDLE thread = CreateThread(NULL, 0, keypress_callback, (void *)callback, 0, NULL);
	return thread;
}


#endif



