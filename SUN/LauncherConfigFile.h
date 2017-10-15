#ifndef SUN_INCLUDE__LAUNCHERCONFIGFILE_H
#define SUN_INCLUDE__LAUNCHERCONFIGFILE_H

class ProcessLauncher;

class LauncherConfigFile
{
	public:
		LauncherConfigFile();
		~LauncherConfigFile();
		bool Read_File(FILE *File); // Actually always returns true, never checked for corruption
		bool Has_Field(char *FieldName, char **ValueOutput);
		bool Get_Game_Executable(ProcessLauncher *UStruct);

	private:
		char *Seek_To_Data(char *Buffer);
		bool Get_Field_Value(char **FieldName, char **Output);
		UINT Get_Field_Xor(char **FieldName);
		void Delete_Field(char **FieldPtr);
		bool Peek_Field_Value(char **Source, char **Destination);
		void Grow(); // Originally didn't exist
		void Shrink(double OldFieldCountRatio); // Originally didn't exist

		char ***_LCFFields;
		int _FieldCount;
		int _CurrentRowCount;
		int _FieldIndexModifier;
		// int _field_10; // Never used
		bool _FixedFieldsSize;
		// int Get_Field_Xor_Ptr;
		double _MinFieldRatio;
		double _MaxFieldRatio;
};

#endif