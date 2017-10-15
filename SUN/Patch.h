#ifndef SUN_INCLUDE__PATCH_H
#define SUN_INCLUDE__PATCH_H

class LauncherConfigFile;

int Get_Patch_File(char *Destination, int DestinationSize, LauncherConfigFile *ConfigFile);
bool Get_SKU_Registry_Path(char *Output, int OutputSize, LauncherConfigFile *ConfigFile, int SKUNumber);
void Patch(char *PatchFilePath, LauncherConfigFile *ConfigFile, int SKUNumber);
INT_PTR Patch_Notes_Window_Function(HWND Window, UINT uMsg, WPARAM wParam, LPARAM lParam);
void Reboot_PC();
void Loop_Trough_Patches(LauncherConfigFile *ConfigFile);

#endif