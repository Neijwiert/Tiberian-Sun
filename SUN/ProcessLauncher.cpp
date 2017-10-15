#include "stdafx.h"
#include "ProcessLauncher.h"
#include <climits>

ProcessLauncher::ProcessLauncher()
{
	this->FirstArgument[0] = '\0';
	this->SecondArgument[0] = '\0';
	this->ThirdArgument[0] = '\0';
	this->Process = NULL;
	this->ProcessThread = NULL;
}

void ProcessLauncher::Create()
{
	STARTUPINFOA PStartupInfo;
	_PROCESS_INFORMATION PInfo;
	char PCMDLineBuffer[512];

	memset(&PStartupInfo, 0, sizeof(STARTUPINFOA));
	memset(PCMDLineBuffer, '\0', sizeof(PCMDLineBuffer));

	PStartupInfo.cb = 68;

	strcpy_s(PCMDLineBuffer, this->SecondArgument);
	strcat_s(PCMDLineBuffer, this->ThirdArgument);

	if (CreateProcessA(NULL, PCMDLineBuffer, NULL, NULL, false, NULL, NULL, NULL, &PStartupInfo, &PInfo) == 0)
	{
		char MSGBuffer[256];
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), LANG_USER_DEFAULT, MSGBuffer, sizeof(MSGBuffer), NULL);
	}

	this->Process = PInfo.hProcess;
	this->ProcessThread = PInfo.hThread;
}

bool ProcessLauncher::Watch_Process(LPDWORD ExitCode)
{
	DWORD WaitReturnVal = WaitForSingleObject(this->Process, UINT_MAX);
	if (ExitCode != NULL)
	{
		*ExitCode = -1;
	}

	if (WaitReturnVal != 0)
	{
		return false;
	}
	else
	{
		if (ExitCode != NULL)
		{
			GetExitCodeProcess(this->Process, ExitCode);
		}

		return true;
	}
}