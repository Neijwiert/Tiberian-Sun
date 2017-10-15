#include "stdafx.h"
#include "PatchDialog.h"
#include "Resource.h"
#include <CommCtrl.h>
#include <ctime>

#define IMAGE_PATH "launcher.bmp"
#define BMP_HEADER_SIZE 14
#define PALETTE_VERSION 768 // DONT KNOW IF THERES AN ENUM FOR THIS
#define PALETTE_ENTRY_COUNT 256 // DONT KNOW IF THERES AN ENUM FOR THIS
#define INIT_BITMAP_BITS 4 // DONT KNOW IF THERES AN ENUM FOR THIS
#define MIN_PROGRESSBAR_RANGE 0
#define MAX_PROGRESSBAR_RANGE 100
#define PROGRESSBAR_STEP 10
#define PATCH_ERROR_LOG_FILENAME "patch.err"

PatchDialog PatchDialog::Instance;
HWND PatchDialog::InstanceHWND = NULL;
bool PatchDialog::InstanceCreated = false;

PatchDialog::PatchDialog()
{
	this->_Image = NULL;
	this->_Palette = NULL;
	this->_ImageComponent = NULL;
	this->_CurrentPatchFile = 0;
	this->_TotalPatchFileCount = 0;
}

PatchDialog::~PatchDialog() // Originally did not exist
{
	DeleteObject(this->_Image);
	DeleteObject(this->_Palette);

	this->_Image = NULL;
	this->_Palette = NULL;
}

bool PatchDialog::Init(const char *ImagePath, HWND ImageComponent)
{
	this->_ImageComponent = ImageComponent;
	HANDLE ImageFile = CreateFileA(ImagePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);

	if (ImageFile != INVALID_HANDLE_VALUE)
	{
		char BMPHeaderBuffer[BMP_HEADER_SIZE];
		DWORD BytesRead;
		BITMAPINFOHEADER BMPInfoHeader;

		ReadFile(ImageFile, BMPHeaderBuffer, BMP_HEADER_SIZE, &BytesRead, NULL); // Skip the header
		ReadFile(ImageFile, &BMPInfoHeader, sizeof(BMPInfoHeader), &BytesRead, NULL);

		// Code assumes a color table is present with max colors count and no compression
		HGLOBAL BMPMemHandle = GlobalAlloc(GHND, sizeof(BMPInfoHeader) + (sizeof(RGBQUAD) * (1u << BMPInfoHeader.biBitCount))); // sizeof(BMPInfoHeader) + (sizeof(RGBQUAD) * (2 ^ BMPInfoHeader.biBitCount)))
		BITMAPINFO *BMPInfo = (BITMAPINFO *)GlobalLock(BMPMemHandle);

		BMPInfo->bmiHeader.biSize = BMPInfoHeader.biSize;
		BMPInfo->bmiHeader.biWidth = BMPInfoHeader.biWidth;
		BMPInfo->bmiHeader.biHeight = BMPInfoHeader.biHeight;
		BMPInfo->bmiHeader.biPlanes = BMPInfoHeader.biPlanes;
		BMPInfo->bmiHeader.biBitCount = BMPInfoHeader.biBitCount;
		BMPInfo->bmiHeader.biCompression = BMPInfoHeader.biCompression;
		BMPInfo->bmiHeader.biSizeImage = BMPInfoHeader.biSizeImage;
		BMPInfo->bmiHeader.biXPelsPerMeter = BMPInfoHeader.biXPelsPerMeter;
		BMPInfo->bmiHeader.biYPelsPerMeter = BMPInfoHeader.biYPelsPerMeter;
		BMPInfo->bmiHeader.biClrUsed = BMPInfoHeader.biClrUsed;
		BMPInfo->bmiHeader.biClrImportant = BMPInfoHeader.biClrImportant;

		ReadFile(ImageFile, BMPInfo->bmiColors, sizeof(RGBQUAD) * (1u << BMPInfoHeader.biBitCount), &BytesRead, NULL); // sizeof(RGBQUAD) * (2 ^ BMPInfoHeader.biBitCount))

		LOGPALETTE *Palette = (LOGPALETTE *)malloc(sizeof(LOGPALETTE) + (sizeof(PALETTEENTRY) * PALETTE_ENTRY_COUNT));
		Palette->palVersion = PALETTE_VERSION;
		Palette->palNumEntries = PALETTE_ENTRY_COUNT;

		for (int EntryIndex = 0; EntryIndex < PALETTE_ENTRY_COUNT; EntryIndex++)
		{
			Palette->palPalEntry[EntryIndex].peRed = BMPInfo->bmiColors[EntryIndex].rgbRed;
			Palette->palPalEntry[EntryIndex].peGreen = BMPInfo->bmiColors[EntryIndex].rgbGreen;
			Palette->palPalEntry[EntryIndex].peBlue = BMPInfo->bmiColors[EntryIndex].rgbBlue;
			Palette->palPalEntry[EntryIndex].peFlags = BMPInfo->bmiColors[EntryIndex].rgbReserved;
		}

		this->_Palette = CreatePalette(Palette);
		free(Palette);

		UINT RowSize = ((BMPInfo->bmiHeader.biBitCount * BMPInfo->bmiHeader.biWidth + 31) / 32) * 4;
		UINT PixelArraySize = RowSize * abs(BMPInfo->bmiHeader.biHeight);

		HGLOBAL BMPPixelDataMemHandle = GlobalAlloc(GHND, PixelArraySize);
		void *BMPPixelData = GlobalLock(BMPPixelDataMemHandle);

		ReadFile(ImageFile, BMPPixelData, PixelArraySize, &BytesRead, NULL);

		HDC ImageDC = GetDC(ImageComponent);
		if (SelectPalette(ImageDC, this->_Palette, false) != NULL)
		{
			if (RealizePalette(ImageDC) != GDI_ERROR)
			{
				HBITMAP Image = CreateDIBitmap(ImageDC, &BMPInfoHeader, INIT_BITMAP_BITS, BMPPixelData, BMPInfo, DIB_RGB_COLORS);

				this->_Image = Image;
				ReleaseDC(ImageComponent, ImageDC);
				if (this->_Image != NULL)
				{
					GlobalUnlock(BMPMemHandle);
					GlobalUnlock(BMPPixelDataMemHandle);
					CloseHandle(ImageFile);

					if (this->_Image != NULL) // Is this necessary?
					{
						RECT ImageRectangle;

						GetClientRect(ImageComponent, &ImageRectangle);
						InvalidateRect(ImageComponent, &ImageRectangle, true);

						UpdateWindow(ImageComponent);

						return true;
					}
				}
			}
		}
	}

	return false;
}

bool PatchDialog::Paint()
{
	if (this->_Image != NULL)
	{
		InvalidateRect(this->_ImageComponent, NULL, false);

		PAINTSTRUCT Paint;
		BeginPaint(this->_ImageComponent, &Paint);

		if (SelectPalette(Paint.hdc, this->_Palette, false) == NULL)
		{
			SHOW_DIALOG_ARGS(IDS_FAIL_SELECT_PALETTE, IDC_OK, MB_OK, GetLastError());
		}
		if (RealizePalette(Paint.hdc) == GDI_ERROR)
		{
			SHOW_DIALOG_ARGS(IDS_FAIL_REALIZE_PALETTE, IDC_OK, MB_OK, GetLastError());
		}

		HDC CompatibleDC = CreateCompatibleDC(Paint.hdc);
		SelectObject(CompatibleDC, this->_Image);

		BITMAP BMP;
		GetObjectA(this->_Image, sizeof(BITMAP), &BMP);

		RECT ImageRectangle;
		GetClientRect(this->_ImageComponent, &ImageRectangle);
		SetStretchBltMode(Paint.hdc, STRETCH_DELETESCANS); // MSDN says STRETCH_DELETESCANS == COLORONCOLOR
		StretchBlt(Paint.hdc, 0, 0, ImageRectangle.right, ImageRectangle.bottom, CompatibleDC, 0, 0, BMP.bmWidth, BMP.bmHeight, SRCCOPY);
		DeleteDC(CompatibleDC);

		EndPaint(this->_ImageComponent, &Paint);

		return true;
	}

	return false;
}

HWND PatchDialog::Create()
{
	InstanceHWND = CreateDialogParamA(Constants.hInstance, (LPCSTR)IDD_PATCH_DIALOG, NULL, (DLGPROC)Window_Function, NULL);

	ShowWindow(InstanceHWND, SW_SHOWNORMAL);
	SetForegroundWindow(InstanceHWND);

	return InstanceHWND;
}

// No idea if it returns anything and no way to check unless I decompile patchw32.dll
// Though most likely it doesn't return anything.
const char *PatchDialog::Patcher_Callback(CallbackType Type, void *Message) 
{
	SUN_UNTESTEDA("Type: %d", Type);

	MSG DispatchedMSG;
	for (int x = 0; x < 100; x++) // DONT KNOW WHY THIS IS HERE
	{
		if (PeekMessageA(&DispatchedMSG, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&DispatchedMSG);
			DispatchMessageA(&DispatchedMSG);
		}
		else
		{
			break;
		}
	}

	FILE *LogFile;
	time_t CurrentTime;

	switch (Type)
	{
		case TYPE_ERROR:
			SHOW_DIALOG_CUSTOM_TEXT((char *)Message, IDC_ERROR, MB_OK);

			fopen_s(&LogFile, PATCH_ERROR_LOG_FILENAME, "a");
			CurrentTime = time(0);

			fprintf_s(LogFile, "\n\nPatch Error: %s\n", Time_To_Local_Time_String(&CurrentTime));
			fprintf_s(LogFile, "%s\n", Message);

			fclose(LogFile);

			return "";
		case TYPE_FILE_PROGRESS_UPDATE:
			DEBUG_MESSAGEA("Message = %u\n", (UINT)Message);

			SendMessageA(GetDlgItem(InstanceHWND, IDDC_PATCH_DIALOG_PROGRESS_BAR), PBM_SETPOS, (UINT)Message, 0); // TODO: Check if Message is indeed between 0-100, currently that's unknown.

			return "";
		case TYPE_INIT:
			Instance._CurrentPatchFile = 0;
			Instance._TotalPatchFileCount = (UINT)Message;

			return "";
		case TYPE_FILE_UPDATE:
			SetWindowTextA(GetDlgItem(InstanceHWND, IDDC_PATCH_DIALOG_FILEINDEX_LABEL), (char *)Message);
			SendMessageA(GetDlgItem(InstanceHWND, IDDC_PATCH_DIALOG_PROGRESS_BAR), PBM_SETPOS, 0, 0);

			Instance._CurrentPatchFile++;

			char FileIndexBuffer[MAX_LOADSTRING];
			LoadStringA(NULL, IDS_FILE_DOWNLOAD_INDICATOR, FileIndexBuffer, MAX_LOADSTRING);

			char Buffer[MAX_LOADSTRING];
			sprintf_s(Buffer, FileIndexBuffer, Instance._CurrentPatchFile, Instance._TotalPatchFileCount);

			SetWindowTextA(GetDlgItem(InstanceHWND, IDDC_PATCH_DIALOG_CURFILE_LABEL), Buffer);

			return "";
		case TYPE_COMPLETE:
			SendMessageA(GetDlgItem(InstanceHWND, IDDC_PATCH_DIALOG_PROGRESS_BAR), PBM_SETPOS, MAX_PROGRESSBAR_RANGE, 0);

			return "";
		default:
			return "";
	}

	return 0;
}

BOOL PatchDialog::Window_Function(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!InstanceCreated)
	{
		InstanceCreated = true;
		Instance = PatchDialog();

		atexit(Delete);
	}

	switch (uMsg)
	{
		case WM_PAINT:
			Instance.Paint();

			return FALSE;
		case WM_CLOSE:
			DestroyWindow(Window);
			PostQuitMessage(0);

			exit(0);

			return FALSE;
		case WM_INITDIALOG:
			SendMessageA(GetDlgItem(Window, IDDC_PATCH_DIALOG_PROGRESS_BAR), PBM_SETRANGE, 0, MIN_PROGRESSBAR_RANGE << 0 | MAX_PROGRESSBAR_RANGE << 16);
			SendMessageA(GetDlgItem(Window, IDDC_PATCH_DIALOG_PROGRESS_BAR), PBM_SETPOS, 0, 0);
			SendMessageA(GetDlgItem(Window, IDDC_PATCH_DIALOG_PROGRESS_BAR), PBM_SETSTEP, PROGRESSBAR_STEP, 0);

			Instance.Init(IMAGE_PATH, GetDlgItem(Window, IDDC_PATCH_DIALOG_IMAGE));

			return TRUE;
		default:
			return FALSE;
	}
}

void PatchDialog::Delete()
{
	Instance.~PatchDialog();

	Instance.InstanceCreated = false; // Original was never reset to false
}