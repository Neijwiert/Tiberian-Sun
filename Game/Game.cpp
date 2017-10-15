// Game.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Game.h"
#include <float.h>

#define GAME_MUTEX_NAME "29e3bb2a-2f36-11d3-a72c-0090272fa661"
#define b350c6d2_MUTEX_NAME "b350c6d2-2f36-11d3-a72c-0090272fa661"
#define LANGUAGE_MODULE_NAME "Language.dll"

bool Load_Language_Lib(bool ShowError)
{
	if (Constants.LanguageModule != NULL || (Constants.LanguageModule = LoadLibraryA(LANGUAGE_MODULE_NAME)) != NULL)
	{
		return true;
	}
	else
	{
		if (ShowError)
		{
			SHOW_DIALOG(IDS_FAILED_INIT_LANG_DLL, IDC_TIBERIAN_SUN, MB_ICONERROR);
		}

		return false;
	}
}

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	Constants.MainInstance = hInstance;
	Constants.MainThreadID = GetCurrentThreadId();

	HANDLE GameMutex = CreateMutexA(NULL, FALSE, GAME_MUTEX_NAME);
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		HWND ExistingGame = FindWindowA(GAME_MUTEX_NAME, NULL);
		if (ExistingGame != NULL)
		{
			SetForegroundWindow(ExistingGame);
			ShowWindow(ExistingGame, SW_RESTORE);
		}

		if (GameMutex != NULL)
		{
			CloseHandle(GameMutex);
			GameMutex = NULL;
		}

		return 0;
	}

	HANDLE b350c6d2Mutex;
	do
	{
		b350c6d2Mutex = OpenMutexA(MUTEX_ALL_ACCESS, FALSE, b350c6d2_MUTEX_NAME);
		if (b350c6d2Mutex != NULL)
		{
			if (WaitForSingleObject(b350c6d2Mutex, 30000u) == WAIT_FAILED)
			{
				CloseHandle(b350c6d2Mutex);
				b350c6d2Mutex = NULL;
			}
		}

		if (b350c6d2Mutex != NULL)
		{
			break;
		}

		b350c6d2Mutex = CreateMutexA(NULL, FALSE, b350c6d2_MUTEX_NAME);
		if (GetLastError() == ERROR_ALREADY_EXISTS)
		{
			CloseHandle(b350c6d2Mutex);
			b350c6d2Mutex = NULL;

			Sleep(2500u);
		}
	} 
	while (b350c6d2Mutex == NULL);

	//_controlfp(RC_CHOP, MCW_RC); // Set floating-point control word // _controlfp_s
	// MAYBE this word is stored, unknown as of now
	// atexit(exit(1)); // INVESTIGATE -> loc_601AB0

	if (!Load_Language_Lib(true))
	{
		return 0;
	}

	if (GetSystemMetrics(SM_MOUSEPRESENT) == 0 || GetSystemMetrics(SM_CMOUSEBUTTONS) == 0)
	{

	}

	return 0;
}
