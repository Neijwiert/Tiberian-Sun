#ifndef GAME_INCLUDE__DEBUG_H
#define GAME_INCLUDE__DEBUG_H

#ifdef _DEBUG
	#define VA_ARGS_BUFFER_SIZE 1024
	#define WIDE2(x) L##x
	#define WIDE1(x) WIDE2(x)
	#define __WFILE__ WIDE1(__FILE__)
	#define __WFUNCSIG__ WIDE1(__FUNCSIG__)

	void Debug_MessageW(const wchar_t *Format, ...);
	void Debug_MessageA(const char *Format, ...);

	#define DEBUG_MESSAGEW(Format, ...)  Debug_MessageW(Format, __VA_ARGS__)
	#define DEBUG_MESSAGEA(Format, ...)  Debug_MessageA(Format, __VA_ARGS__)

	#define SUN_UNTESTEDA(Format, ...)																																			\
	{																																											\
		char Buffer[VA_ARGS_BUFFER_SIZE];																																		\
		sprintf_s(Buffer, VA_ARGS_BUFFER_SIZE, Format, __VA_ARGS__);																											\
																																												\
		if (strlen(Format) == 0)																																				\
		{																																										\
			Debug_MessageA("Executing untested function\nFilename: %s\nFunction name: %s\nLine number: %u\n", __FILE__, __FUNCSIG__, __LINE__);									\
		}																																										\
		else																																									\
		{																																										\
			Debug_MessageA("Executing untested function\nFilename: %s\nFunction name: %s\nLine number: %u\nArguments: %s\n", __FILE__, __FUNCSIG__, __LINE__, Buffer);			\
		}																																										\
	}

	#define SUN_UNTESTEDW(Format, ...)																																			\
	{																																											\
		wchar_t Buffer[VA_ARGS_BUFFER_SIZE];																																	\
		swprintf_s(Buffer, VA_ARGS_BUFFER_SIZE, Format, __VA_ARGS__);																											\
																																												\
		if (wcslen(Format) == 0)																																				\
		{																																										\
			Debug_MessageW(L"Executing untested function\nFilename: %ls\nFunction name: %ls\nLine number: %u\n", __WFILE__, __WFUNCSIG__, __LINE__);							\
		}																																										\
		else																																									\
		{																																										\
			Debug_MessageW(L"Executing untested function\nFilename: %ls\nFunction name: %ls\nLine number: %u\nArguments: %ls\n", __WFILE__, __WFUNCSIG__, __LINE__, Buffer);	\
		}																																										\
	}
#else
	#define VA_ARGS_BUFFER_SIZE
	#define WIDE2(x)
	#define WIDE1(x)
	#define __WFILE__
	#define __WFUNCSIG__
	#define DEBUG_MESSAGEW(Format, ...)
	#define DEBUG_MESSAGEA(Format, ...)
	#define SUN_UNTESTEDA(Format, ...)
	#define SUN_UNTESTEDW(Format, ...)
#endif

#endif