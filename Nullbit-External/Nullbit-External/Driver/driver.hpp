#pragma once
#include <Windows.h>
#include <cstdint>
#include <TlHelp32.h>
#include <string>
#include <iostream>
#include "../Roblox/Globals.h"

namespace driver {
    namespace codes {
        constexpr ULONG attach = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x696, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG read = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x697, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
        constexpr ULONG write = CTL_CODE(FILE_DEVICE_UNKNOWN, 0x698, METHOD_BUFFERED, FILE_SPECIAL_ACCESS);
    }

    struct Request {
        HANDLE process_id;
        PVOID target;
        PVOID buffer;
        SIZE_T size;
        SIZE_T return_size;
    };

    
}

bool attach_to_process(const DWORD pid)
{
    driver::Request r;
    r.process_id = reinterpret_cast<HANDLE>(pid);

    return DeviceIoControl(globals::hProc, driver::codes::attach, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);
}

template <class T>
T read(uintptr_t addr) {
	HANDLE hProc = globals::hProc;
    T temp = {};
    
    driver::Request r;
	r.target = reinterpret_cast<PVOID>(addr);
	r.buffer = &temp;
	r.size = sizeof(T);

    DeviceIoControl(hProc, driver::codes::read, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

    return temp;
}

template <class T>
void write(uintptr_t addr, T value) {
    HANDLE hProc = globals::hProc;
	driver::Request r;
	r.target = reinterpret_cast<PVOID>(addr);
	r.buffer = (PVOID)&value;
	r.size = sizeof(T);

    DeviceIoControl(hProc, driver::codes::write, &r, sizeof(r), &r, sizeof(r), nullptr, nullptr);

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
