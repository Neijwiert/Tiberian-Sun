// SUN.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SUN.h"
#include <CommCtrl.h>
#include <direct.h>
#include "LauncherConfigFile.h"
#include "ProcessLauncher.h"
#include "Patch.h"

#pragma comment( lib, "Comctl32.lib" )

#define PATCHER_FILE_NAME "patchget.dat"

int Run_SUN(int ArgumentCount, char *ModuleFileName);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	Constants.CommandLine = WChar_To_Char(lpCmdLine); // No support for UNICODE yet
	Constants.WindowShowType = nCmdShow;
	Constants.hInstance = hInstance;

	char ModuleFileName[512];

	GetModuleFileNameA(hInstance, ModuleFileName, 512);

	int ArgumentCount = 1;
	int CommandLineCharIndex = 0;
	char CurrentCMDLineChar = '\0';

	do // Not 100% sure this command line parsing is correct
	{
		do
		{
			CurrentCMDLineChar = Constants.CommandLine[CommandLineCharIndex++];
		} while (CurrentCMDLineChar == ' ');

		if (CurrentCMDLineChar == '\0')
		{
			break;
		}
		else if (CurrentCMDLineChar != '\r')
		{
			ArgumentCount++;

			do
			{
				CurrentCMDLineChar = Constants.CommandLine[CommandLineCharIndex++];
			} while (CurrentCMDLineChar != ' ' && CurrentCMDLineChar != '\0' && CurrentCMDLineChar != '\r');

			Constants.CommandLine[CommandLineCharIndex - 1] = '\0';
		}
	} 
	while (CurrentCMDLineChar != '\0' && CurrentCMDLineChar != '\r' && ArgumentCount < 20);

	return Run_SUN(ArgumentCount, ModuleFileName);
}

int Run_SUN(int ArgumentCount, char *ModuleFileName)
{
	char CurrentWorkingDir[252];

	_getcwd(CurrentWorkingDir, 252);

	InitCommonControls(); // Investigate InitCommonControlsEx

	Set_Work_Directory(ModuleFileName);

	char LCFPath[512]; // Actual = 256 bytes
	strcpy_s(LCFPath, ModuleFileName);

	char *LastDot = LCFPath;
	for (char *Ptr = strchr(LCFPath + 1, '.'); Ptr != NULL; Ptr = strchr(Ptr + 1, '.'))
	{
		LastDot = Ptr;
	}

	if (LastDot[0] == '.')
	{
		LastDot[0] = '\0';
	}

	memcpy(LastDot, ".lcf", sizeof(char) * 4);

	LauncherConfigFile ConfigFile;

	FILE *LCFHandle = NULL;
	fopen_s(&LCFHandle, LCFPath, "r");

	if (LCFHandle == NULL)
	{
		SHOW_DIALOG(IDS_RUN_FROM_INSTALL_DIR, IDC_RUN_FROM_INSTALL_DIR, MB_OK);

		exit(-1);
	}

	bool Success = ConfigFile.Read_File(LCFHandle); // Always returns true

	fclose(LCFHandle);

	if (!Success)
	{
		SHOW_DIALOG(IDS_LCF_CORRUPT, IDC_ERROR, MB_OK);

		exit(-1);
	}

	ProcessLauncher GameExe;
	ConfigFile.Get_Game_Executable(&GameExe);

	if (ArgumentCount > 1)
	{
		MessageBoxA(NULL, "ArgumentCount > 1", "NO CAPTION", MB_OK);
		
		// TODO: Find out what parsing is going on to the command line
	}
	else if (ArgumentCount < 2 || strcmp(ModuleFileName + 1, "GrabPatches") != 0)
	{
		char PatchFilePath[260];
		bool PatchRequired = true;

		while (true)
		{
			int PatchFileSKUNumber = Get_Patch_File(PatchFilePath, 260, &ConfigFile);
			if (PatchFileSKUNumber != 0)
			{
				while (PatchFileSKUNumber)
				{
					Patch(PatchFilePath, &ConfigFile, PatchFileSKUNumber);

					PatchFileSKUNumber = Get_Patch_File(PatchFilePath, 260, &ConfigFile);
				} 
			}
			else
			{
				if (!PatchRequired)
				{
					Loop_Trough_Patches(&ConfigFile); // This affects absolutely nothing
					Set_Work_Directory(CurrentWorkingDir);

					return 0;
				}
			}

			PatchRequired = false;

			Set_Work_Directory(ModuleFileName);

			DWORD GameExitCode;

			GameExe.Create();
			GameExe.Watch_Process(&GameExitCode);

			if (GameExitCode == 123456789)
			{
				PatchRequired = true;

				ProcessLauncher PatcherDat;

				strcpy_s(PatcherDat.FirstArgument, GameExe.FirstArgument);
				strcpy_s(PatcherDat.SecondArgument, PATCHER_FILE_NAME);
				strcpy_s(PatcherDat.ThirdArgument, "");

				PatcherDat.Create();
				PatcherDat.Watch_Process(NULL);
			}
		}
	}

	ProcessLauncher PatcherDat;

	strcpy_s(PatcherDat.FirstArgument, GameExe.FirstArgument);
	strcpy_s(PatcherDat.SecondArgument, PATCHER_FILE_NAME);
	strcpy_s(PatcherDat.ThirdArgument, "");

	PatcherDat.Create();
	PatcherDat.Watch_Process(NULL);

	char PatchFilePath[260];
	for (int CurrentSKUNumber = Get_Patch_File(PatchFilePath, sizeof(PatchFilePath), &ConfigFile); CurrentSKUNumber != 0; CurrentSKUNumber = Get_Patch_File(PatchFilePath, sizeof(PatchFilePath), &ConfigFile))
	{
		Patch(PatchFilePath, &ConfigFile, CurrentSKUNumber);
	}

	Set_Work_Directory(CurrentWorkingDir);

	return 0;
}

