#include <iostream>
#include <Windows.h>
#include <cmath>
#include <ctime>
#include <string>
#include <stdio.h>
#include <TlHelp32.h>
#include <tchar.h>
#include <time.h>
#include <conio.h>


using namespace std;

// Functions
DWORD_PTR dwGetModuleBaseAddress(DWORD dwProcID, const TCHAR *szModuleName);
void WriteToMemory(HANDLE handle, DWORD addressToWrite, DWORD value);
DWORD FindDmaAddy(int PointerLevel, HANDLE handle, DWORD OFfsets[], DWORD BaseAddress);
void SendKeyPress();
uintptr_t GetModuleBaseAddress(DWORD dwProcID, const TCHAR *szModuleName);


// Game thingys
string GameName = "AssaultCube";
LPCSTR LGameWindow = "AssaultCube";
string GameStatus;

HANDLE handle;
DWORD PID;

/* -------------------------------------OFFSETS --------------------------------- */

DWORD LocalPlayer = 0x0051E20C;
DWORD EntityBase_Offset = 0x110D90;
DWORD EntityLoopDistance = 0x4;

// No Recoil & No Kickback

bool noRecoilStatus;
DWORD noRecoilBaseAddress = { 0x463786 };

// Triggerbot

bool triggerStatus;

DWORD text_write = 0;
DWORD shootValue_on = 1634628865;
DWORD shootValue_off = 1634628864;
DWORD shootOffsets[] = { 0x224 };
DWORD shootAddress; // LocalPlayer + 0x224
DWORD targetNameAddress = 0x501C38;

// RapidFire

bool rapidStatus;

DWORD rapidOffsets_knife[] = {0x160};
DWORD rapidAddress_knife;
DWORD rapidOffsets_pistol[] = {0x164};
DWORD rapidAddress_pistol;
DWORD rapidOffsets_rifle[] = {0x178};
DWORD rapidAddress_rifle;
DWORD rapidOffsets_grenades[] = {0x180};
DWORD rapidAddress_grenades;
DWORD rapidValue = 0;

// PlayerAmount

DWORD playerAmountAddress = 0x50F500;
DWORD playerAmount = 0;



struct MyPlayer 
{
	int Team;
	DWORD TargetName;

	void ReadInformation()
	{
		ReadProcessMemory(handle, (PBYTE*)(LocalPlayer + 0x32C), &Team, sizeof(Team), 0);

		ReadProcessMemory(handle, (LPCVOID)targetNameAddress, &TargetName, sizeof(TargetName), NULL);
		/*if (text_read != 0)
		{
			WriteProcessMemory(handle, (BYTE*)targetNameAddress, &text_write, sizeof(text_write), NULL);
		}*/

	}

}MyPlayer;


struct Enemy 
{
	DWORD CBaseEntity = (GetModuleBaseAddress(PID, _T("ac_client.exe"))) + EntityBase_Offset;
	int Team;
	DWORD name;

	void ReadInformation(int Player)
	{
		CBaseEntity = CBaseEntity + (Player*EntityLoopDistance);

		ReadProcessMemory(handle, (PBYTE*)(CBaseEntity + 0x32C), &Team, sizeof(Team), 0);

		ReadProcessMemory(handle, (PBYTE*)(CBaseEntity + 0x225), &name, sizeof(name), 0);
	}

}PlayerList[32];


int main() {

	HWND hwnd = NULL;

	while (hwnd == NULL) // Searching for games window
	{
		hwnd = FindWindow(NULL, LGameWindow);
		Sleep(10);
	}

	cout << "Game has been found!" << endl;

	PID = NULL;	
	GetWindowThreadProcessId(hwnd, &PID); //Getting the games process id

	if (PID != NULL)
	{
		cout << "Process ID has been successfully found!" << endl;
	}
	else
	{
		cout << "Process ID has been failed to found!" << endl;
	}

	handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID); // Opening the process with full rights
	if (handle == INVALID_HANDLE_VALUE || handle == NULL) 
	{
		cout << "Failed to open process with all access!" << endl;
	}

	

	shootAddress = FindDmaAddy(1, handle, shootOffsets, LocalPlayer);

	rapidAddress_grenades = FindDmaAddy(1, handle, rapidOffsets_grenades, LocalPlayer);
	rapidAddress_knife = FindDmaAddy(1, handle, rapidOffsets_knife, LocalPlayer);
	rapidAddress_pistol = FindDmaAddy(1, handle, rapidOffsets_pistol, LocalPlayer);
	rapidAddress_rifle = FindDmaAddy(1, handle, rapidOffsets_rifle, LocalPlayer);

	while (!GetAsyncKeyState(VK_INSERT))
	{

		system("cls");
		cout << "-----------------------------------------------" << endl;
		cout << "                 AssaultCubev1.x               " << endl;
		cout << "-----------------------------------------------" << endl;
		cout << "[F1] Triggerbot Status: " << triggerStatus << endl;
		cout << "[F2] NoRecoil & NoKickback Status: " << noRecoilStatus << endl;
		cout << "[F3] RapidFire Status: " << rapidStatus << endl;
		cout << "[INSERT] Exit" << endl;

		if (GetAsyncKeyState(VK_F1))
			triggerStatus = !triggerStatus;

		if (GetAsyncKeyState(VK_F2))
		{
			noRecoilStatus = !noRecoilStatus;
			if (noRecoilStatus)
				WriteProcessMemory(handle, (BYTE*)noRecoilBaseAddress, "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 10, NULL);
			else
				WriteProcessMemory(handle, (BYTE*)noRecoilBaseAddress, "\x50\x8D\x4C\x24\x1C\x51\x8B\xCE\xFF\xD2", 10, NULL);
		}

		if (GetAsyncKeyState(VK_F3))
			rapidStatus = !rapidStatus;

		if (triggerStatus)
		{
			MyPlayer.ReadInformation();
			ReadProcessMemory(handle, (LPCVOID)playerAmountAddress, &playerAmount, sizeof(playerAmount), NULL);
			
			for (int i = 0; i < playerAmount; i++)
			{
				PlayerList[i].ReadInformation(i);

			}

			/*if (MyPlayer.TargetName != 0 & MyPlayer.Team != PlayerList[i].Team)
			{
				WriteToMemory(handle, shootAddress, shootValue_on);
				Sleep(100);
				WriteToMemory(handle, shootAddress, shootValue_off);
			}*/
			
		}


		if (rapidStatus)
		{
			WriteToMemory(handle, rapidAddress_grenades, rapidValue);
			WriteToMemory(handle, rapidAddress_knife, rapidValue);
			WriteToMemory(handle, rapidAddress_pistol, rapidValue);
			WriteToMemory(handle, rapidAddress_rifle, rapidValue);
		}


		Sleep(1);		
	}

	CloseHandle(handle);
	CloseHandle(hwnd);

	return ERROR_SUCCESS;
}










DWORD FindDmaAddy(int PointerLevel, HANDLE handle, DWORD Offsets[], DWORD BaseAddress)
{
	DWORD pointer = BaseAddress;
	DWORD pTemp;

	DWORD pointerAddr;

	for (int i = 0; i < PointerLevel; i++)
	{
		if (i == 0)
		{
			ReadProcessMemory(handle, (LPCVOID)pointer, &pTemp, sizeof(pTemp), NULL);
		}
		pointerAddr = pTemp + Offsets[i];
		ReadProcessMemory(handle, (LPCVOID)pointerAddr, &pTemp, sizeof(pTemp), NULL);
	}
	return pointerAddr;
}

void WriteToMemory(HANDLE handle, DWORD addressToWrite, DWORD value)
{
	WriteProcessMemory(handle, (BYTE*)addressToWrite, &value, sizeof(value), NULL);
}

void SendKeyPress()
{
	INPUT ip;

	ip.type = INPUT_MOUSE;
	ip.ki.time = 0;
	ip.ki.wVk = 0;
	ip.ki.dwExtraInfo = 0;
	ip.ki.dwFlags = KEYEVENTF_SCANCODE;
	ip.ki.wScan = 0;
	SendInput(1, &ip, sizeof(INPUT));
	Sleep(25);
	ip.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &ip, sizeof(INPUT));
}


DWORD_PTR dwGetModuleBaseAddress(DWORD dwProcID, const TCHAR *szModuleName)
{
	DWORD_PTR dwModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (_tcsicmp(ModuleEntry32.szModule, szModuleName) == 0)
				{
					dwModuleBaseAddress = (DWORD_PTR)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));
		}
		CloseHandle(hSnapshot);
	}
	return dwModuleBaseAddress;
}



uintptr_t GetModuleBaseAddress(DWORD dwProcID, const TCHAR *szModuleName)
{
	uintptr_t ModuleBaseAddress = 0;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcID);
	if (hSnapshot != INVALID_HANDLE_VALUE)
	{
		MODULEENTRY32 ModuleEntry32;
		ModuleEntry32.dwSize = sizeof(MODULEENTRY32);
		if (Module32First(hSnapshot, &ModuleEntry32))
		{
			do
			{
				if (_tcsicmp(ModuleEntry32.szModule, szModuleName) == 0)
				{
					ModuleBaseAddress = (uintptr_t)ModuleEntry32.modBaseAddr;
					break;
				}
			} while (Module32Next(hSnapshot, &ModuleEntry32));
		}
		CloseHandle(hSnapshot);
	}
	return ModuleBaseAddress;
}
