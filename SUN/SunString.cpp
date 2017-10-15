#include "stdafx.h"
#include "SunString.h"

void Clear_Pointer(char **Ptr)
{
	*Ptr = NULL;
}

/*
void Delete_PointerA(char **Ptr)
{
	Delete_PointerB(Ptr);
}
*/

void Delete_Pointer(char **Ptr)
{
	if (*Ptr != NULL)
	{
		delete[] * Ptr;
	}

	*Ptr = NULL;
}

char *WChar_To_Char(const wchar_t *WStr)
{
	if (WStr == NULL)
	{
		char *Str = new char[2];
		Str[0] = '\0';
		Str[1] = '\0';

		return Str;
	}

	int WStrLength = wcslen(WStr);
	char *Str = new char[WStrLength + 1];

	size_t ConvertedChars;
	wcstombs_s(&ConvertedChars, Str, WStrLength + 1, WStr, WStrLength + 1);

	return Str;
}

UINT Safe_String_Length(char **Ptr)
{
	if (*Ptr != NULL)
	{
		return strlen(*Ptr);
	}
	else
	{
		return 0u;
	}
}

char *Safe_Get_String(char **Ptr)
{
	char *Result = *Ptr;
	if (*Ptr == NULL)
	{
		Result = "";
	}

	return Result;
}

/*
void Copy_Data_A(char **Destination, char *Data)
{
	Copy_Data_B(Destination, Data);
}
*/

bool Copy_String(char **Destination, char *Source)
{
	Delete_Pointer(Destination);

	int SourceLength = strlen(Source);
	char *SourceCopy = new char[SourceLength + 1];

	*Destination = SourceCopy;

	if (SourceCopy != NULL)
	{
		memcpy(SourceCopy, Source, SourceLength);
		SourceCopy[SourceLength] = '\0';

		return true;
	}
	else
	{
		return false;
	}
}

bool Copy_String(char **Destination, int SourceSize, char *Source)
{
	Delete_Pointer(Destination);

	char *SourceCopy = new char[SourceSize + 1];
	*Destination = SourceCopy;

	if (SourceCopy != NULL)
	{
		memcpy(SourceCopy, Source, SourceSize); // Originally strncpy
		(*Destination)[SourceSize] = '\0';

		return true;
	}
	else
	{
		return false;
	}
}

bool Crop_String(char **Str, int NewSize)
{
	char *Tmp;

	Clear_Pointer(&Tmp);

	char *SafeTmp;
	char *SafeStr = Safe_Get_String(Str);

	if (Copy_String(&Tmp, NewSize, SafeStr) && (SafeTmp = Safe_Get_String(&Tmp), Copy_String(Str, SafeTmp)))
	{
		Delete_Pointer(&Tmp);

		return true;
	}
	else
	{
		Delete_Pointer(&Tmp);

		return false;
	}

	return true;
}

bool Safe_String_Compare(char **Str1, char **Str2)
{
	if (*Str1 != NULL || *Str2 != NULL)
	{
		if (*Str1 != NULL && *Str2 != NULL)
		{
			return (strcmp(*Str1, *Str2) == 0);
		}
		else
		{
			return false;
		}
	}
	else
	{
		return true;
	}
}

char **Safe_String_Copy(char **Destination, char **Source) // Not sure if it returns anything
{
	if (!Safe_String_Compare(Destination, Source))
	{
		Copy_String(Destination, Safe_Get_String(Source));
	}

	return Destination;
}

bool Crop_String_To_End(char **Str, int StartIndex, int EndIndex) // Not 100% this function is correct
{
	if (*Str != NULL)
	{
		int StrLen = strlen(*Str);
		if (StartIndex + EndIndex > StrLen)
		{
			StartIndex = StrLen - EndIndex;
		}

		if (StartIndex < 0)
		{
			EndIndex += StartIndex;
			StartIndex = 0;
		}

		if (EndIndex > 0)
		{
			char *NewStr = new char[StrLen - EndIndex + 1];
			if (NewStr != NULL)
			{
				(*Str)[StartIndex] = '\0';
				*(&(*Str)[EndIndex - 1] + StartIndex) = '\0';

				memcpy(NewStr, *Str + StartIndex + EndIndex, StrLen - EndIndex);
				NewStr[StrLen - EndIndex] = '\0';

				delete[] * Str;
				*Str = NewStr;

				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

int Split_String(char **Str, UINT StartIndex, const char *DelimStr, char **Output)
{
	UINT StrLen = Safe_String_Length(Str);

	for (; StartIndex < StrLen; StartIndex++)
	{
		if (!strchr(DelimStr, (*Str)[StartIndex]))
		{
			break;
		}
	}

	if (StartIndex < StrLen)
	{
		UINT BeginIndex = StartIndex;
		for (; StartIndex < StrLen; StartIndex++)
		{
			if (strchr(DelimStr, (*Str)[StartIndex]))
			{
				break;
			}
		}

		Copy_String(Output, &(*Str)[BeginIndex]);
		Crop_String(Output, StartIndex - BeginIndex);

		return StartIndex;
	}
	else
	{
		return -1;
	}
}

bool Crop_String_To_Char(char **Str, char SeperatorChar)
{
	if (*Str != NULL)
	{
		char *SeparatorPointer = strchr(*Str, SeperatorChar);
		if (SeparatorPointer != NULL)
		{
			Crop_String(Str, SeparatorPointer - *Str);

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void Remove_Spaces(char **Str)
{
	Remove_Character(Str, ' ');
	Remove_Character(Str, '\t');
}

bool Remove_Character(char **Str, char CharToRemove)
{
	if (*Str != NULL)
	{
		int DestStrLength = strlen(*Str);
		char *CharPtr = strchr(*Str, CharToRemove);

		if (CharPtr != NULL)
		{
			do
			{
				memcpy(CharPtr, CharPtr + 1, *Str - CharPtr + DestStrLength-- - 1);
				(*Str)[DestStrLength] = '\0';
				CharPtr = strchr(*Str, CharToRemove);
			} 
			while (CharPtr != NULL);

			char *NewValueDest = new char[strlen(*Str) + 1];
			memcpy(NewValueDest, *Str, strlen(*Str));
			NewValueDest[strlen(*Str)] = '\0';

			delete[](*Str);
			*Str = NewValueDest;

			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

void To_Upper(char **Str)
{
	int DestStrLength = Safe_String_Length(Str);

	if (DestStrLength > 0)
	{
		for (int x = 0; x < DestStrLength; x++)
		{
			char CurChar = (*Str)[x];
			if (CurChar >= 'a' && CurChar <= 'z')
			{
				(*Str)[x] = toupper(CurChar);
			}
		}
	}
}