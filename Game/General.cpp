#include "stdafx.h"
#include "General.h"

GeneralConstants GeneralConstants::Instance = GeneralConstants();

char TimeBuffer[512];

GeneralConstants::GeneralConstants()
{
	this->MainInstance = NULL;
	this->MainThreadID = 0;
	this->LanguageModule = NULL;
}