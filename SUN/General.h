#ifndef SUN_INCLUDE__GENERAL_H
#define SUN_INCLUDE__GENERAL_H

struct GeneralConstants;

#define Constants GeneralConstants::Instance

#define SHOW_DIALOG(Text, Caption, Buttons)								\
{																		\
	char TextBuffer[MAX_LOADSTRING];									\
	char CaptionBuffer[MAX_LOADSTRING];									\
																		\
	LoadStringA(NULL, Text, TextBuffer, MAX_LOADSTRING);				\
	LoadStringA(NULL, Caption, CaptionBuffer, MAX_LOADSTRING);			\
																		\
	MessageBoxA(NULL, TextBuffer, CaptionBuffer, Buttons);				\
}

#define SHOW_DIALOG_ARGS(Text, Caption, Buttons, ...)					\
{																		\
	char TextBuffer[MAX_LOADSTRING];									\
	char CaptionBuffer[MAX_LOADSTRING];									\
																		\
	LoadStringA(NULL, Text, TextBuffer, MAX_LOADSTRING);				\
	LoadStringA(NULL, Caption, CaptionBuffer, MAX_LOADSTRING);			\
																		\
	char SprintfBuffer[MAX_LOADSTRING];									\
	sprintf_s(SprintfBuffer, MAX_LOADSTRING, TextBuffer, __VA_ARGS__);	\
																		\
	MessageBoxA(NULL, SprintfBuffer, CaptionBuffer, Buttons);			\
}

#define SHOW_DIALOG_CUSTOM_TEXT(Text, Caption, Buttons)					\
{																		\
	char CaptionBuffer[MAX_LOADSTRING];									\
																		\
	LoadStringA(NULL, Caption, CaptionBuffer, MAX_LOADSTRING);			\
																		\
	MessageBoxA(NULL, Text, CaptionBuffer, Buttons);					\
}

struct GeneralConstants
{
public:
	static GeneralConstants Instance;

	char *CommandLine;
	int WindowShowType;
	HINSTANCE hInstance;

private:
	GeneralConstants();
};

const char *Time_To_Local_Time_String(time_t *Time);
int Set_Work_Directory(char *ModuleFileName);

#endif