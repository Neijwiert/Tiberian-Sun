#ifndef SUN_INCLUDE__SUNSTRING_H
#define SUN_INCLUDE__SUNSTRING_H

void Clear_Pointer(char **Ptr);
// void Delete_PointerA(char **Ptr); // Obsolete
void Delete_Pointer(char **Ptr);
char *WChar_To_Char(const wchar_t *WStr);
char *Safe_Get_String(char **Ptr);
// void Copy_Data_A(char **Destination, char *Data); // Obsolete
bool Copy_String(char **Destination, char *Source);
bool Copy_String(char **Destination, int SourceSize, char *Source);
bool Crop_String(char **Str, int NewSize);
UINT Safe_String_Length(char **Ptr);
bool Safe_String_Compare(char **Str1, char **Str2);
char **Safe_String_Copy(char **Destination, char **Source);
bool Crop_String_To_End(char **Str, int StartIndex, int EndIndex);
int Split_String(char **Str, UINT StartIndex, const char *DelimStr, char **Output);
bool Crop_String_To_Char(char **Str, char SeperatorChar); 
void Remove_Spaces(char **Str); 
bool Remove_Character(char **Str, char CharToRemove); 
void To_Upper(char **Str);

#endif SUN_INCLUDE__SUNSTRING_H