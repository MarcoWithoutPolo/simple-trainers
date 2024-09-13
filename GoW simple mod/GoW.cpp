#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <tchar.h> // _tcscmp
#include <vector>
#include <limits.h>

uintptr_t GetModuleBaseAddress(TCHAR* lpszModuleName, DWORD pID) {
    uintptr_t modBaseAddr = 0;
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, pID);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        std::cerr << "Failed to create toolhelp32 snapshot." << std::endl;
        return 0;
    }

    MODULEENTRY32 ModuleEntry32 = { sizeof(MODULEENTRY32) };
    if (Module32First(hSnapshot, &ModuleEntry32)) {
        do {
            if (_tcscmp(ModuleEntry32.szModule, lpszModuleName) == 0) {
                modBaseAddr = reinterpret_cast<uintptr_t>(ModuleEntry32.modBaseAddr);
                break;
            }
        } while (Module32Next(hSnapshot, &ModuleEntry32));
    }

    CloseHandle(hSnapshot);
    return modBaseAddr;
}

int main() {
    HWND hwnd = FindWindowA(NULL, "God of War");
    if (hwnd == NULL) {
        std::cerr << "Cannot find window" << std::endl;
        Sleep(1000);
        return -1;
    }

    DWORD procID;
    GetWindowThreadProcessId(hwnd, &procID);
    std::cout << "Process ID: " << procID << std::endl;
    if (procID == NULL) {
        std::cerr << "Failed to get process ID." << std::endl;
        Sleep(1000);
        return -1;
    }

    HANDLE handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (handle == NULL) {
        std::cerr << "Failed to open process." << std::endl;
        Sleep(1000);
        return -1;
    }

    TCHAR gameName[] = _T("GoW.exe");

    uintptr_t gameBaseAddress = GetModuleBaseAddress(gameName, procID);
    if (gameBaseAddress == 0) {
        std::cerr << "Failed to get base address of the module." << std::endl;
        Sleep(1000);
        CloseHandle(handle);
        return -1;
    }

    DWORD offsetGameToBaseAddress = 0x01426420;

    //Offsets
    std::vector<DWORD> xpOffsets{ 0x30 };
    std::vector<DWORD> hcOffsets{ 0x1F0 };
    std::vector<DWORD> mistOffsets{ 0x370, 0x8, 0x20 };

    uintptr_t baseAddress = 0;

    if (!ReadProcessMemory(handle, (LPVOID)(gameBaseAddress + offsetGameToBaseAddress), &baseAddress, sizeof(baseAddress), NULL)) {
        std::cerr << "Failed to read memory." << std::endl;
        Sleep(1000);
        CloseHandle(handle);
        return -1;
    }

    std::cout << "Base address: " << std::hex << baseAddress << std::endl;

    //Find the address for the XP
    uintptr_t xpAddress = baseAddress;
    for (size_t i = 0; i < xpOffsets.size() - 1; i++) {
        if (!ReadProcessMemory(handle, (LPVOID)(xpAddress + xpOffsets.at(i)), &xpAddress, sizeof(xpAddress), NULL)) {
            std::cerr << "Failed to read memory at offset." << std::endl;
            Sleep(1000);
            CloseHandle(handle);
            return -1;
        }
    }
    xpAddress += xpOffsets.at(xpOffsets.size() - 1);

    //Find the address for the Hacksilver
    uintptr_t hcAddress = baseAddress;
    for (size_t i = 0; i < hcOffsets.size() - 1; i++) {
        if (!ReadProcessMemory(handle, (LPVOID)(hcAddress + hcOffsets.at(i)), &xpAddress, sizeof(hcAddress), NULL)) {
            std::cerr << "Failed to read memory at offset." << std::endl;
            Sleep(1000);
            CloseHandle(handle);
            return -1;
        }
    }
    hcAddress += hcOffsets.at(hcOffsets.size() - 1);

    //Mist freezer
    DWORD mistoffsetGameToBaseAddress = 0x011BE5C8;
    if (!ReadProcessMemory(handle, (LPVOID)(gameBaseAddress + mistoffsetGameToBaseAddress), &baseAddress, sizeof(baseAddress), NULL)) {
        std::cerr << "Failed to read memory." << std::endl;
        Sleep(1000);
        CloseHandle(handle);
        return -1;
    }
    uintptr_t mistAddress = baseAddress;
    for (size_t i = 0; i < mistOffsets.size() - 1; i++) {
        if (!ReadProcessMemory(handle, (LPVOID)(mistAddress + mistOffsets.at(i)), &mistAddress, sizeof(mistAddress), NULL)) {
            std::cerr << "Failed to read memory at offset." << std::endl;
            Sleep(1000);
            CloseHandle(handle);
            return -1;
        }
    }
    mistAddress += mistOffsets.at(mistOffsets.size() - 1);

    std::cout << "Initial Mist Value: " << mistAddress << std::endl;

    bool freezeMist = false;
    int mistValue = 0;

    std::cout << "GoW xp hack" << std::endl;
    std::cout << "Press Numpad 0 to EXIT" << std::endl;
    std::cout << "Press F2 to modify xp" << std::endl;
    std::cout << "Press F3 to modify Hacksilver" << std::endl;
    std::cout << "Press F4 to freeze mist timer" << std::endl;


    while (true) {
        Sleep(50);
        if (GetAsyncKeyState(VK_NUMPAD0)) { // Exit
            CloseHandle(handle);
            return 0;
        }

        //XP input
        if (GetAsyncKeyState(VK_F2)) {

            int newXp = 0;

            
            std::cout << "Enter the new XP value: ";
            std::cin >> newXp;
            if (!WriteProcessMemory(handle, (LPVOID)(xpAddress), &newXp, sizeof(newXp), 0)) {
                std::cerr << "Failed to write memory." << std::endl;
            }
        }

        //Hacksilver input
        if (GetAsyncKeyState(VK_F3)) {
            int newHc = 0;
           
            std::cout << "Enter the new HS value: ";
            std::cin >> newHc;
            if (!WriteProcessMemory(handle, (LPVOID)(hcAddress), &newHc, sizeof(newHc), 0)) {
                std::cerr << "Failed to write memory." << std::endl;
            }
        }

        //Mist updater
        if (GetAsyncKeyState(VK_F4)) {
            if (!freezeMist) {
                freezeMist = true;
                if (!ReadProcessMemory(handle, (LPVOID)(mistAddress), &mistValue, sizeof(mistValue), NULL)) {
                    std::cerr << "Failed to read memory." << std::endl;
                    Sleep(1000);
                    CloseHandle(handle);
                    return -1;
                }
                std::cout << "Mist Timer Frozen at: " << mistValue << std::endl;
            }
            else {
                freezeMist = false;
                std::cout << "Mist Timer Unfrozen" << std::endl;
            }
        }

        if (freezeMist) {
            if (!WriteProcessMemory(handle, (LPVOID)(mistAddress), &mistValue, sizeof(mistValue), 0)) {
                std::cerr << "Failed to write memory." << std::endl;
            }
        }

            




            
        
    }

    CloseHandle(handle);
    return 0;
}