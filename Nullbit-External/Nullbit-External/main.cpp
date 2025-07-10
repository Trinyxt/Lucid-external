#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>

//#include "offsets.hpp"
#include "RobloxFunc.hpp"
#include "MemUtils.hpp"

const std::wstring& roblox = L"RobloxPlayerBeta.exe";

int main() {
	DWORD ProcessID = GetProcId(roblox);
	if (ProcessID == 0) {
		std::cout << "Make sure Roblox is opened!" << std::endl;
		return 1;
	}

	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, ProcessID);
	if (!hProc) {
		std::cerr << "Failed to open process handle. Error: " << GetLastError() << "\n";
		return -1;
	}

	uintptr_t baseAddress = GetModuleBaseAddy(ProcessID, roblox);

	if (baseAddress == 0) {
		std::cerr << "Failed to get module base address." << std::endl;
		CloseHandle(hProc);
		return -1;
	}

	uintptr_t datamodel = findDatamodel(hProc, baseAddress,true);
	uintptr_t playerService = findPlayerService(hProc, datamodel, true);
	uintptr_t localPlayer = findLocalPlayer(hProc, playerService, true);
	uintptr_t character = findChar(hProc, localPlayer, true);
	uintptr_t humanoid = findHumanoid(hProc, character, true);

	float WalkSpeed = read<float>(hProc, humanoid + offsets::WalkSpeed);
	std::cout << "WalkSpeed: " << WalkSpeed << std::endl;

	float speed = 100;

	write<float>(hProc, humanoid + offsets::WalkSpeed, speed);
	write<float>(hProc, humanoid + offsets::WalkSpeedCheck, speed);

	std::cout << "New walkspeed: " << read<float>(hProc, humanoid + offsets::WalkSpeed) << std::endl;
}