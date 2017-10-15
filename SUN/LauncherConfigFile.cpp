#include "stdafx.h"
#include "LauncherConfigFile.h"
#include "ProcessLauncher.h"

#define COMMENT_CHARACTER '#'
#define FIELD_NAME_VALUE_SEPARATOR '='
#define MIN_LCF_FIELDS 32
#define START_FIELD_INDEX_MODIFIER 5
#define GAME_EXECUTABLE_FIELD_NAME "RUN"

/*
	When changing MIN_LCF_FIELDS you have to either multiply or divide it by 2.
	And when you do so, this rule applies:
	Multiplying: increase START_FIELD_INDEX_MODIFIER by one
	Dividing: decrease START_FIELD_INDEX_MODIFIER by one
*/

LauncherConfigFile::LauncherConfigFile()
{
	// Each row in _LCFFields consists of 3 char * pointers. 
	// Index 0: Field Name
	// Index 1: Field Value
	// Index 2: When there's a duplicate entry, the duplicate one is stored in this index. This pointer must be cast to a char ** which in turn is of the same format as what is described here.
	this->_LCFFields = (char ***)malloc(MIN_LCF_FIELDS * sizeof(char **));
	this->_FieldCount = 0;
	this->_CurrentRowCount = MIN_LCF_FIELDS;
	this->_FieldIndexModifier = START_FIELD_INDEX_MODIFIER;
	// this->_field_10 = 0;
	this->_FixedFieldsSize = false;
	this->_MinFieldRatio = 0.2;
	this->_MaxFieldRatio = 0.8;

	memset(this->_LCFFields, 0, MIN_LCF_FIELDS * sizeof(char **));

	for (UINT x = 0; x < MIN_LCF_FIELDS; x++)
	{
		this->_LCFFields[x] = NULL;
	}
}

LauncherConfigFile::~LauncherConfigFile()
{
	if (this->_CurrentRowCount > 0)
	{
		for (int x = 0; x < this->_CurrentRowCount; x++)
		{
			char **CurrentRow = this->_LCFFields[x];
			while (CurrentRow != NULL)
			{
				char **CurrentDuplicate = (char **)CurrentRow[2];

				Delete_Pointer(&CurrentRow[0]);
				Delete_Pointer(&CurrentRow[1]);

				delete[] CurrentRow;

				CurrentRow = CurrentDuplicate;
			}

			this->_LCFFields[x] = NULL;
		}
	}

	int OldRowCount = this->_CurrentRowCount;
	for (this->_FieldCount = 0; OldRowCount > MIN_LCF_FIELDS; OldRowCount = this->_CurrentRowCount) // Isn't this leaking memory?
	{
		if (this->_FixedFieldsSize)
		{
			break;
		}
		else if (OldRowCount > MIN_LCF_FIELDS)
		{
			char ***OldLCFFields = this->_LCFFields;
			this->_CurrentRowCount /= 2;
			this->_FieldIndexModifier--;

			this->_LCFFields = (char ***)malloc(this->_CurrentRowCount * sizeof(char **));
			memset(this->_LCFFields, 0, this->_CurrentRowCount * sizeof(char **));

			for (int x = 0; x < this->_CurrentRowCount; x++)
			{
				this->_LCFFields[x] = NULL;
			}

			for (int x = 0; x < OldRowCount; x++)
			{
				char **CurrentField = OldLCFFields[x];
				if (CurrentField != NULL)
				{
					char **DuplicatePtr;
					do
					{
						int Index = Get_Field_Xor(&CurrentField[0]) & ((1 << this->_FieldIndexModifier) - 1);
						char **MatchField = this->_LCFFields[Index];

						this->_LCFFields[Index] = CurrentField;
						DuplicatePtr = (char **)CurrentField[2];
						CurrentField[2] = (char *)MatchField;
						CurrentField = DuplicatePtr;
					} 
					while (DuplicatePtr != NULL);
				}
			}

			free(OldLCFFields);
		}
	}

	free(this->_LCFFields);
}

bool LauncherConfigFile::Read_File(FILE *File)
{
	char *FieldName;
	char *FieldValue;

	Clear_Pointer(&FieldName);
	Clear_Pointer(&FieldValue);

	char Buffer[256];
	memset(Buffer, '\0', sizeof(Buffer));

	while (fgets(Buffer, sizeof(Buffer), File) != NULL)
	{
		char *Data = Seek_To_Data(Buffer);

		if (Data[0] != '\0' && Data[0] != COMMENT_CHARACTER && strchr(Data, FIELD_NAME_VALUE_SEPARATOR) != NULL)
		{
			Copy_String(&FieldName, Data);
			Crop_String_To_Char(&FieldName, FIELD_NAME_VALUE_SEPARATOR);
			Remove_Spaces(&FieldName);
			To_Upper(&FieldName);

			char *SeparatorPtr = strchr(Data, FIELD_NAME_VALUE_SEPARATOR);
			Data = Seek_To_Data(SeparatorPtr + 1);

			Copy_String(&FieldValue, Data);
			Crop_String_To_Char(&FieldValue, '\r');
			Crop_String_To_Char(&FieldValue, '\n');

			char **FieldCopy = new char *[3];
			if (FieldCopy != NULL)
			{
				Clear_Pointer(&FieldCopy[0]);
				Clear_Pointer(&FieldCopy[1]);
			}

			Safe_String_Copy(&FieldCopy[0], &FieldName);
			Safe_String_Copy(&FieldCopy[1], &FieldValue);
			FieldCopy[2] = NULL;

			char *StoredFieldValue;
			Clear_Pointer(&StoredFieldValue);

			Get_Field_Value(&FieldName, &StoredFieldValue);

			Delete_Pointer(&StoredFieldValue);

			int Index = ((1 << this->_FieldIndexModifier) - 1) & Get_Field_Xor(&FieldName);

			DEBUG_MESSAGEA("Loaded field [%s] with value [%s] from the LCF, storing at index %d\n", FieldName, FieldValue, Index);

			if (this->_LCFFields[Index] != NULL)
			{
				FieldCopy[2] = (char *)this->_LCFFields[Index];
			}

			this->_LCFFields[Index] = FieldCopy;

			Grow();
		}
	}

	Delete_Pointer(&FieldName);
	Delete_Pointer(&FieldValue);

	return true;
}

bool LauncherConfigFile::Has_Field(char *FieldName, char **ValueOutput)
{
	char *Buffer;
	Clear_Pointer(&Buffer);

	Copy_String(&Buffer, FieldName);

	bool Result = Peek_Field_Value(&Buffer, ValueOutput);

	Delete_Pointer(&Buffer);

	return Result;
}

bool LauncherConfigFile::Get_Game_Executable(ProcessLauncher *UStruct)
{
	char *Buffer;
	Clear_Pointer(&Buffer);

	if (Has_Field(GAME_EXECUTABLE_FIELD_NAME, &Buffer))
	{
		char *StrPtr2;
		char *StrPtr3;
		char *StrPtr4;

		Clear_Pointer(&StrPtr2);
		Clear_Pointer(&StrPtr3);
		Clear_Pointer(&StrPtr4);

		int FirstArgumentIndex = Split_String(&Buffer, 0, " ", &StrPtr4); 
		int SecondArgumentIndex = Split_String(&Buffer, FirstArgumentIndex, " ", &StrPtr3);
		Safe_String_Copy(&StrPtr2, &Buffer);

		Crop_String_To_End(&StrPtr2, 0, SecondArgumentIndex);

		strcpy_s(UStruct->FirstArgument, Safe_Get_String(&StrPtr4));
		strcpy_s(UStruct->SecondArgument, Safe_Get_String(&StrPtr3));
		strcpy_s(UStruct->ThirdArgument, Safe_Get_String(&StrPtr2));

		Delete_Pointer(&StrPtr2);
		Delete_Pointer(&StrPtr3);
		Delete_Pointer(&StrPtr4);
		Delete_Pointer(&Buffer);

		return true;
	}
	else
	{
		Delete_Pointer(&Buffer);

		return false;
	}
}

char *LauncherConfigFile::Seek_To_Data(char *Buffer)
{
	char *ResultPointer = Buffer;
	if (isspace(ResultPointer[0]))
	{
		int CurChar;
		do
		{
			CurChar = (ResultPointer++)[1];
		} 
		while (isspace(CurChar));
	}

	return ResultPointer;
}

UINT LauncherConfigFile::Get_Field_Xor(char **FieldName)
{
	UINT StrLen = Safe_String_Length(FieldName);
	UINT Result = StrLen;

	for (UINT x = 0; x < StrLen; x++)
	{
		char *Str = Safe_Get_String(FieldName);
		Result = ((x + Str[x] + Result) >> 24) ^ ((x + Str[x] + Result) << 8);
	}

	return Result;
}

bool LauncherConfigFile::Get_Field_Value(char **FieldName, char **Output)
{
	if (this->_FieldCount == 0)
	{
		return false;
	}
	
	int Index = Get_Field_Xor(FieldName) & ((1 << this->_FieldIndexModifier) - 1);

	char **LCFRow = this->_LCFFields[Index];
	if (LCFRow == NULL)
	{
		return false;
	}

	double OldFieldCountRatio = (double)(this->_FieldCount - 1) / (double)this->_CurrentRowCount;
	if (Safe_String_Compare(&LCFRow[0], FieldName))
	{
		Safe_String_Copy(Output, &LCFRow[1]);
		
		char **DuplicatePtr = (char **)LCFRow[2];

		Delete_Field(LCFRow);
		delete[] LCFRow;

		this->_LCFFields[Index] = DuplicatePtr;
		this->_FieldCount--;

		Shrink(OldFieldCountRatio);
	}
	else
	{
		char **DuplicatePtr = (char **)LCFRow[2];
		if (DuplicatePtr != NULL)
		{
			bool GotMatch = true;
			while (!Safe_String_Compare(&DuplicatePtr[0], FieldName))
			{
				LCFRow = DuplicatePtr;
				DuplicatePtr = (char **)DuplicatePtr[2];
				if (DuplicatePtr == NULL)
				{
					GotMatch = false;
				}
			}

			if (GotMatch)
			{
				Safe_String_Copy(Output, &DuplicatePtr[1]);
				LCFRow[2] = DuplicatePtr[2];
				this->_FieldCount--;

				if (DuplicatePtr != NULL)
				{
					Delete_Field(DuplicatePtr);
					delete[] DuplicatePtr;
				}
			}
		}

		Shrink(OldFieldCountRatio);
	}

	return true;
}

void LauncherConfigFile::Delete_Field(char **FieldPtr)
{
	Delete_Pointer(&FieldPtr[0]);
	Delete_Pointer(&FieldPtr[1]);
}

bool LauncherConfigFile::Peek_Field_Value(char **FieldName, char **Output)
{
	int Index = Get_Field_Xor(FieldName) & ((1 << this->_FieldIndexModifier) - 1);
	char **CurrentRow = this->_LCFFields[Index];

	if (CurrentRow != NULL)
	{
		while (true)
		{
			if (Safe_String_Compare(&CurrentRow[0], FieldName))
			{
				break;
			}

			CurrentRow = (char **)CurrentRow[2];
			if (CurrentRow == NULL)
			{
				return false;
			}
		}

		if (CurrentRow != NULL)
		{
			Safe_String_Copy(Output, &CurrentRow[1]);

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

void LauncherConfigFile::Grow()
{
	if ((double)++this->_FieldCount / (double)this->_CurrentRowCount < this->_MaxFieldRatio || this->_FixedFieldsSize)
	{
		return;
	}

	char ***OldLCFFields = this->_LCFFields;
	this->_FieldIndexModifier++;
	int OldRowCount = this->_CurrentRowCount;
	this->_CurrentRowCount *= 2;

	this->_LCFFields = (char ***)malloc(this->_CurrentRowCount * sizeof(char **));
	memset(this->_LCFFields, 0, this->_CurrentRowCount * sizeof(char **));

	for (int x = 0; x < this->_CurrentRowCount; x++)
	{
		this->_LCFFields[x] = NULL;
	}

	for (int x = 0; x < OldRowCount; x++)
	{
		char **CurrentField = OldLCFFields[x];
		if (CurrentField != NULL)
		{
			char **DuplicatePtr;
			do
			{
				int Index = ((1 << this->_FieldIndexModifier) - 1) & Get_Field_Xor(&CurrentField[0]);
				char **MatchField = this->_LCFFields[Index];

				this->_LCFFields[Index] = CurrentField;
				DuplicatePtr = (char **)CurrentField[2];
				CurrentField[2] = (char *)MatchField;
				CurrentField = DuplicatePtr;
			} 
			while (DuplicatePtr != NULL);
		}
	}

	free(OldLCFFields);
}

void LauncherConfigFile::Shrink(double OldFieldCountRatio)
{
	if (OldFieldCountRatio > this->_MinFieldRatio || this->_CurrentRowCount <= MIN_LCF_FIELDS || this->_FixedFieldsSize)
	{
		return;
	}

	char ***OldLCFFields = this->_LCFFields;
	this->_FieldIndexModifier--;
	int OldRowCount = this->_CurrentRowCount;
	this->_CurrentRowCount /= 2;

	this->_LCFFields = (char ***)malloc(this->_CurrentRowCount * sizeof(char **));
	memset(this->_LCFFields, 0, this->_CurrentRowCount * sizeof(char **));

	for (int x = 0; x < this->_CurrentRowCount; x++)
	{
		this->_LCFFields[x] = NULL;
	}

	for (int x = 0; x < OldRowCount; x++)
	{
		char **CurrentField = OldLCFFields[x];
		if (CurrentField != NULL)
		{
			char **DuplicatePtr;
			do
			{
				int Index = Get_Field_Xor(&CurrentField[0]) & ((1 << this->_FieldIndexModifier) - 1);
				char **MatchField = this->_LCFFields[Index];

				this->_LCFFields[Index] = CurrentField;
				DuplicatePtr = (char **)CurrentField[2];
				CurrentField[2] = (char *)MatchField;
				CurrentField = DuplicatePtr;
			} 
			while (DuplicatePtr != NULL);
		}
	}

	free(OldLCFFields);
}