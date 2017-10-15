#include "stdafx.h"
#include "General.h"
#include <time.h>
#include <direct.h>

GeneralConstants GeneralConstants::Instance = GeneralConstants();

char TimeBuffer[512];

GeneralConstants::GeneralConstants()
{
	this->CommandLine = NULL;
	this->WindowShowType = 0;
	this->hInstance = NULL;
}

const char *Time_To_Local_Time_String(time_t *Time)
{
	tm LocalTime;

	if (localtime_s(&LocalTime, Time) == 0)
	{
		if (asctime_s(TimeBuffer, &LocalTime) == 0)
		{
			return TimeBuffer;
		}
		else
		{
			return NULL;
		}
	}
	else
	{
		return NULL;
	}
}

int Set_Work_Directory(char *ModuleFileName)
{
	char Drive[12];
	char Dir[256];
	char Filename[256];
	char Ext[63];
	char RunDirectory[516];

	_splitpath_s(ModuleFileName, Drive, Dir, Filename, Ext);
	_makepath_s(RunDirectory, Drive, Dir, NULL, NULL);

	if (RunDirectory[strlen(ModuleFileName) - 1] == '\\')
	{
		RunDirectory[strlen(ModuleFileName) - 1] = '\0';
	}

	char UppercaseDriveLetter = toupper(RunDirectory[0]);
	int Result = _chdrive(UppercaseDriveLetter - 64);

	if (!Result)
	{
		Result = _chdir(RunDirectory);
	}

	return Result;
}