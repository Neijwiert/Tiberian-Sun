#include "stdafx.h"
#include "Debug.h"
#include <stdarg.h>

#ifdef _DEBUG

void Debug_MessageW(const wchar_t *Format, ...)
{
	wchar_t Buffer[VA_ARGS_BUFFER_SIZE];
	va_list Arg;
	va_start(Arg, Format);

	_vsnwprintf_s(Buffer, VA_ARGS_BUFFER_SIZE, Format, Arg);

	va_end(Arg);

	OutputDebugStringW(Buffer);
}

void Debug_MessageA(const char *Format, ...)
{
	char Buffer[VA_ARGS_BUFFER_SIZE];
	va_list Arg;
	va_start(Arg, Format);

	_vsnprintf_s(Buffer, VA_ARGS_BUFFER_SIZE, Format, Arg);

	va_end(Arg);

	OutputDebugStringA(Buffer);
}

#endif