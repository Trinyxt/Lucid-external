#pragma once
#include <Windows.h>
#include <cstdint>
#include <TlHelp32.h>
#include <string>
#include <iostream>

template <typename T>
T read(HANDLE hProc, uintptr_t addr) {
    T buffer{};
    ReadProcessMemory(hProc, (LPCVOID)addr, &buffer, sizeof(T), nullptr);
    return buffer;
}

template <typename T>
bool write(HANDLE hProc, uintptr_t addr, T value) {
    return WriteProcessMemory(hProc, (LPVOID)addr, &value, sizeof(T), nullptr);
}

DWORD GetProcId(const std::wstring& procName) {
    DWORD procId = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry = { sizeof(entry) };
    if (Process32FirstW(hSnap, &entry)) {
        do {
            if (!_wcsicmp(entry.szExeFile, procName.c_str())) {
                procId = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(hSnap, &entry));
    }
    CloseHandle(hSnap);
    return procId;
}

uintptr_t GetModuleBaseAddy(DWORD procId, const std::wstring& modName) {
    uintptr_t modBase = 0;
    HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, procId);
    MODULEENTRY32W modEntry = { sizeof(modEntry) };
    if (Module32FirstW(hSnap, &modEntry)) {
        do {
            if (!_wcsicmp(modEntry.szModule, modName.c_str())) {
                modBase = (uintptr_t)modEntry.modBaseAddr;
                break;
            }
        } while (Module32NextW(hSnap, &modEntry));
    }
    CloseHandle(hSnap);
    return modBase;
}

/*uintptr_t FindAddress(HANDLE hProc, uintptr_t ptr, const std::string& addressName, bool shouldPrint) {
    uintptr_t address = read<uintptr_t>(hProc, ptr);

    if (address == 0) {
        std::cerr << "Failed to get " << addressName << " address." << std::endl;
        CloseHandle(hProc);
        return 0;
    }
    Sleep(1);
    if (shouldPrint == true)
        std::cout << addressName << " address is: " << std::hex << address << std::endl;

    return address;
}*/
