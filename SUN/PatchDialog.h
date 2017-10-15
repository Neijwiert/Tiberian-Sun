#ifndef SUN_INCLUDE__PATCHDIALOG_H
#define SUN_INCLUDE__PATCHDIALOG_H

enum CallbackType
{
	TYPE_ERROR = 4,
	TYPE_FILE_PROGRESS_UPDATE,
	TYPE_INIT,
	TYPE_FILE_UPDATE,
	TYPE_COMPLETE
};

class PatchDialog
{
	public:
		PatchDialog();
		~PatchDialog(); // Originally did not exist, it was a global static function
		bool Init(const char *ImagePath, HWND ImageComponent);
		bool Paint();

		static HWND Create();
		static const char *Patcher_Callback(CallbackType Type, void *Message);

	private:
		HGDIOBJ _Image;
		HPALETTE _Palette;
		HWND _ImageComponent;
		UINT _CurrentPatchFile; // Not originally in this structure
		UINT _TotalPatchFileCount; // Not originally in this structure

		static BOOL Window_Function(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam);
		static void Delete();

		static PatchDialog Instance;
		static HWND InstanceHWND;
		static bool InstanceCreated;
};

#endif