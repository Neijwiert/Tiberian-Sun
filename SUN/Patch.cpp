#include "stdafx.h"
#include "Patch.h"
#include <direct.h>
#include <shellapi.h>
#include <ctime>
#include "LauncherConfigFile.h"
#include "ProcessLauncher.h"
#include "PatchDialog.h"
#include "Resource.h"

#define WEB_EXTENSION ".web"
#define EXE_EXTENSION ".exe"
#define EXN_EXTENSION ".exn"
#define RTP_EXTENSION ".rtp"
#define PATCH_LIBRARY "patchw32.dll"
#define PATCH_APPLY_INIT "RTPatchApply32@12"
#define PATCH_NOTES_FILENAME "launcher.txt"
#define REG_KEY_INSTALL_PATH "InstallPath"
#define REG_KEY_EXE_PATCH_FILE "EXEPatch"
#define REG_KEY_VERSION "Version"

#define HAS_EXTENSION(Path, Extension) (strcmp(Path + strlen(Path) - strlen(Extension), Extension) == 0)

typedef const char *(*PatcherCallback) (CallbackType Type, void *Message);
typedef void(*PatcherInit) (char *StartupCommand, PatcherCallback Callback, int Unknown);

bool DoneReadingPatchNotes = false;

bool Get_SKU_Registry_Path(char *Output, int OutputSize, LauncherConfigFile *ConfigFile, int SKUNumber)
{
	char *SKUValuePtr;
	char *SKUPathPtr;
	char *Tmp;

	Clear_Pointer(&SKUValuePtr);
	Clear_Pointer(&SKUPathPtr);
	Clear_Pointer(&Tmp);

	char SKUFieldBuffer[128];
	sprintf_s(SKUFieldBuffer, "SKU%d", SKUNumber);
	if (!ConfigFile->Has_Field(SKUFieldBuffer, &SKUValuePtr))
	{
		Delete_Pointer(&SKUValuePtr);
		Delete_Pointer(&SKUPathPtr);
		Delete_Pointer(&Tmp);

		return false;
	}

	int SplitIndex = Split_String(&SKUValuePtr, 0, " ", &Tmp);
	Safe_String_Copy(&SKUPathPtr, &SKUValuePtr);
	Crop_String_To_End(&SKUPathPtr, 0, SplitIndex);

	while (Safe_Get_String(&SKUPathPtr)[0] == ' ')
	{
		Crop_String_To_End(&SKUPathPtr, 0, 1);
	}

	char *SKUPath = Safe_Get_String(&SKUPathPtr);
	HKEY Key;
	DWORD Type;
	BYTE Data[260];
	DWORD DataBufferSize = sizeof(Data);

	// MSDN says KEY_READ == KEY_EXECUTE, The read one seemed the most logical to me
	if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, SKUPath, NULL, KEY_READ, &Key) != ERROR_SUCCESS ||
		RegQueryValueExA(Key, REG_KEY_INSTALL_PATH, NULL, &Type, Data, &DataBufferSize) != ERROR_SUCCESS ||
		Type != 1) // TODO: Investigate what Type 1 is
	{
		Delete_Pointer(&SKUValuePtr);
		Delete_Pointer(&SKUPathPtr);
		Delete_Pointer(&Tmp);

		return false;
	}

	char *SUNFileNamePtr = (char *)Data;
	for (char *Pch = strchr((char *)Data, '\\'); Pch != NULL; Pch = strchr(Pch + 1, '\\'))
	{
		SUNFileNamePtr = Pch + 1;
	}

	if (SUNFileNamePtr != NULL)
	{
		SUNFileNamePtr[0] = '\0';
	}

	strncpy_s(Output, OutputSize, (char *)Data, strlen((char *)Data));

	Delete_Pointer(&SKUValuePtr);
	Delete_Pointer(&SKUPathPtr);
	Delete_Pointer(&Tmp);

	return true;
}

int Get_Patch_File(char *Destination, int DestinationSize, LauncherConfigFile *ConfigFile)
{
	char *PatchExtensions[4] =
	{
		"web",
		"exe",
		"exn",
		"rtp"
	};

	if (PatchExtensions == NULL) // Original had a do-while construction, which just kept going until it failed to find a path.
	{
		return 0;
	}

	int SKUNumber = 1;
	char SKURegPathBuffer[260];
	WIN32_FIND_DATAA PathFindFileData;
	HANDLE FindFileHandle;

	while (true)
	{
		if (!Get_SKU_Registry_Path(SKURegPathBuffer, sizeof(SKURegPathBuffer), ConfigFile, SKUNumber++))
		{
			return 0;
		}

		for (int CurExtensionOffset = 1; CurExtensionOffset < (sizeof(PatchExtensions) / sizeof(char *)); CurExtensionOffset++) // Not in original one, but this one is safe.
		{
			_chdir(SKURegPathBuffer);

			char PatchPath[128];
			sprintf_s(PatchPath, "patches\\*.%s", *(PatchExtensions + CurExtensionOffset));

			FindFileHandle = FindFirstFileA(PatchPath, &PathFindFileData);
			if (FindFileHandle != INVALID_HANDLE_VALUE)
			{
				break;
			}
		}

		if (FindFileHandle != INVALID_HANDLE_VALUE)
		{
			break;
		}
	}

	_getcwd(Destination, DestinationSize);

	strcat_s(Destination, DestinationSize, "\\patches\\");
	strcat_s(Destination, DestinationSize, PathFindFileData.cFileName);

	FindClose(FindFileHandle);

	return SKUNumber;
}

void Patch(char *PatchFilePath, LauncherConfigFile *ConfigFile, int SKUNumber)
{
	SUN_UNTESTEDA("PatchFilePath: %s, SKUNumber: %d", PatchFilePath, SKUNumber);

	if (HAS_EXTENSION(PatchFilePath, EXE_EXTENSION))
	{
		HKEY RebootKey;
		DWORD Disposition;

		if (RegCreateKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\RunOnce", 0, "", REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &RebootKey, &Disposition) != ERROR_SUCCESS)
		{
			SHOW_DIALOG_ARGS(IDS_FAIL_REBOOT_PATCH_INSTALL, IDC_ERROR, MB_OK, PatchFilePath);
		}
		else
		{
			RegSetValueExA(RebootKey, REG_KEY_EXE_PATCH_FILE, 0, REG_SZ, (BYTE *)PatchFilePath, strlen(PatchFilePath) + 1);

			SHOW_DIALOG(IDS_COMPUTER_RESTART, IDC_COMPUTER_RESTART, MB_OK);

			Reboot_PC();
		}

		return;
	}

	if (HAS_EXTENSION(PatchFilePath, RTP_EXTENSION))
	{
		HWND PatchWindow = PatchDialog::Create();
		MSG DispatchedMSG;

		while (PeekMessageA(&DispatchedMSG, NULL, 0, 0, PM_REMOVE)) // DONT KNOW WHY THIS IS HERE
		{
			TranslateMessage(&DispatchedMSG);
			DispatchMessageA(&DispatchedMSG);
		}

		HMODULE PatcherHandle = LoadLibraryA(PATCH_LIBRARY);
		if (PatcherHandle == NULL)
		{
			SHOW_DIALOG_ARGS(IDS_UNABLE_TO_FIND_FILE, IDC_ERROR, MB_OK, PATCH_LIBRARY);

			exit(-1);
		}

		PatcherInit PInit = (PatcherInit)GetProcAddress(PatcherHandle, PATCH_APPLY_INIT);
		if (PInit != NULL)
		{
			char CommandLineBuffer[256];
			sprintf_s(CommandLineBuffer, "\"%s\" .", PatchFilePath);

			PInit(CommandLineBuffer, PatchDialog::Patcher_Callback, 1);

			FreeLibrary(PatcherHandle);
			_unlink(PatchFilePath);
			DestroyWindow(PatchWindow);

			char *VersionStrPtr = NULL;
			for (char *Pch = strchr(PatchFilePath, '\\'); Pch != NULL; Pch = strchr(Pch + 1, '\\'))
			{
				VersionStrPtr = Pch + 1;
			}

			DWORD PatchVersion = 0;
			if (VersionStrPtr != NULL)
			{
				PatchVersion = atol(VersionStrPtr);
			}

			char *SKUSubKey;
			char SKUBuffer[256];
			Clear_Pointer(&SKUSubKey);
			sprintf_s(SKUBuffer, "SKU%d", SKUNumber);

			if (ConfigFile->Has_Field(SKUBuffer, &SKUSubKey))
			{
				char *TmpBuffer1;
				char *TmpBuffer2;

				Clear_Pointer(&TmpBuffer1);
				Clear_Pointer(&TmpBuffer2);

				int SplitIndex = Split_String(&SKUSubKey, 0, " ", &TmpBuffer1);
				Safe_String_Copy(&TmpBuffer2, &SKUSubKey);
				Crop_String_To_End(&TmpBuffer2, 0, SplitIndex);

				while (Safe_Get_String(&TmpBuffer2)[0] == ' ')
				{
					Crop_String_To_End(&TmpBuffer2, 0, 1);
				}

				HKEY VersionRegKey;
				RegOpenKeyExA(HKEY_LOCAL_MACHINE, Safe_Get_String(&TmpBuffer2), 0, KEY_ALL_ACCESS, &VersionRegKey);
				RegSetValueExA(VersionRegKey, REG_KEY_VERSION, 0, REG_DWORD, (BYTE *)&PatchVersion, sizeof(DWORD));

				DoneReadingPatchNotes = false; // Original not reset here.
				DialogBoxParamA(Constants.hInstance, (LPCSTR)IDD_UPDATE_INFORMATION, NULL, (DLGPROC)Patch_Notes_Window_Function, 0);

				Delete_Pointer(&TmpBuffer1);
				Delete_Pointer(&TmpBuffer2);
				Delete_Pointer(&SKUSubKey);
			}
			else
			{
				#ifdef _DEBUG
					time_t CurrentTime = time(0);
					DEBUG_MESSAGEA("ERR %s [D:\\dev\\launcher\\patch.cpp 237] SKU is missing from config file!\n", Time_To_Local_Time_String(&CurrentTime)); // This is an original debug message from source code, only then it was using 'cout'
				#endif

				Delete_Pointer(&SKUSubKey);
			}
		}
		else
		{
			SHOW_DIALOG(IDS_DLL_CORRUPT, IDC_ERROR, MB_OK);
		}

		return;
	}

	if (HAS_EXTENSION(PatchFilePath, EXN_EXTENSION))
	{
		ProcessLauncher PatchProcess;
		strcpy_s(PatchProcess.FirstArgument, ".");
		strcpy_s(PatchProcess.SecondArgument, PatchFilePath);

		PatchProcess.Create();
		PatchProcess.Watch_Process(NULL);

		_unlink(PatchFilePath);

		return;
	}

	if (HAS_EXTENSION(PatchFilePath, WEB_EXTENSION))
	{
		SHOW_DIALOG(IDS_DOWNLOAD_PATCH, IDC_DOWNLOAD_PATCH, MB_OK);

		FILE *PatchFile;
		fopen_s(&PatchFile, PatchFilePath, "r");
		if (PatchFile != NULL)
		{
			char PatchCommand[256];
			fgets(PatchCommand, sizeof(PatchCommand) - 1, PatchFile);
			fclose(PatchFile);

			ShellExecuteA(NULL, NULL, PatchCommand, NULL, ".", SW_SHOW);

			_unlink(PatchFilePath);

			exit(0);
		}

		SHOW_DIALOG_CUSTOM_TEXT(PatchFilePath, IDC_PATCHFILE_MISSING, MB_OK);
	}
}

INT_PTR Patch_Notes_Window_Function(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (uMsg == WM_COMMAND || uMsg == WM_CLOSE)
	{
		if (wParam == 1 || uMsg == WM_CLOSE)
		{
			EndDialog(Window, 0);

			return TRUE;
		}
	}
	else if (uMsg == WM_INITDIALOG)
	{
		FILE *PatchNotesFile;
		fopen_s(&PatchNotesFile, PATCH_NOTES_FILENAME, "r");
		if (PatchNotesFile == NULL)
		{
			EndDialog(Window, -1);

			return TRUE;
		}

		char Buffer[255];
		DWORD EndSelectPos = 0;

		while (fgets(Buffer, sizeof(Buffer), PatchNotesFile) != NULL)
		{
			// There might be some logic missing here, but it seems that this gives the same result

			SendDlgItemMessageA(Window, IDDC_PATCH_NOTES_EDITTEXT, EM_SETSEL, EndSelectPos, EndSelectPos); // Move to end
			SendDlgItemMessageA(Window, IDDC_PATCH_NOTES_EDITTEXT, EM_REPLACESEL, FALSE, (LPARAM)&Buffer); // Append text
			SendDlgItemMessageA(Window, IDDC_PATCH_NOTES_EDITTEXT, EM_GETSEL, NULL, (LPARAM)&EndSelectPos); // Get new end point
		}

		DoneReadingPatchNotes = true;
		fclose(PatchNotesFile);

		return TRUE;
	}
	else if (uMsg == WM_PAINT)
	{
		if (DoneReadingPatchNotes)
		{
			SendDlgItemMessageA(Window, IDDC_PATCH_NOTES_EDITTEXT, EM_SETSEL, -1, 0); // Deselect everything
		}

		DoneReadingPatchNotes = false;

		return FALSE;
	}

	return FALSE;
}

void Reboot_PC()
{
	HANDLE TokenHandle;
	OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &TokenHandle);

	TOKEN_PRIVILEGES NewState;
	LookupPrivilegeValueA(NULL, "SeShutdownPrivilege", &NewState.Privileges[0].Luid);

	NewState.PrivilegeCount = 1;
	NewState.Privileges[0].Attributes = 2;

	AdjustTokenPrivileges(TokenHandle, FALSE, &NewState, 0, NULL, NULL);
	GetLastError();

	if (ExitWindowsEx(EWX_REBOOT, 0) == 0)
	{
		SHOW_DIALOG(IDS_COMPUTER_RESTART_NOW, IDC_OK, MB_OK);

		exit(0);
	}

	MSG DispatchedMSG;
	while (GetMessageA(&DispatchedMSG, NULL, 0, 0))
	{
		TranslateMessage(&DispatchedMSG);
		DispatchMessageA(&DispatchedMSG);
	}
}

void Loop_Trough_Patches(LauncherConfigFile *ConfigFile)
{
	SUN_UNTESTEDA("");

	int CurrentSKUNumber = 1;
	char SKURegPath[260];

	for (bool RegPathResult = Get_SKU_Registry_Path(SKURegPath, sizeof(SKURegPath), ConfigFile, CurrentSKUNumber); RegPathResult; RegPathResult = Get_SKU_Registry_Path(SKURegPath, sizeof(SKURegPath), ConfigFile, ++CurrentSKUNumber))
	{
		if (strlen(SKURegPath) >= 3) // Must be atleast 3 chars long
		{
			strcat_s(SKURegPath, "patches\\*.*");

			WIN32_FIND_DATAA FileData;
			HANDLE FindFileHandle = FindFirstFileA(SKURegPath, &FileData);

			for (; FindFileHandle != INVALID_HANDLE_VALUE; FindNextFileA(FindFileHandle, &FileData))
			{
				while (FindNextFileA(FindFileHandle, &FileData));
			}

			FindClose(FindFileHandle);
		}
	}
}