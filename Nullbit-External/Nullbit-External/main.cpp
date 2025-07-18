#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>

#include "FindAdrress.hpp"
#include "ModifyRobloxMem.hpp"
#include "imguiWindow.hpp"
#include "Roblox/Globals.h"
#include "Driver/driver.hpp"

using namespace std;

void EnableVTMode() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	DWORD dwMode = 0;
	GetConsoleMode(hOut, &dwMode);
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
}

const wstring& roblox = L"RobloxPlayerBeta.exe";

void printLogoandcredts() {
	EnableVTMode();
	cout << "\033[94m"
R"( _                 _        _        ______  __________________
( (    /||\     /|( \      ( \      (  ___ \ \__   __/\__   __/
|  \  ( || )   ( || (      | (      | (   ) )   ) (      ) (   
|   \ | || |   | || |      | |      | (__/ /    | |      | |   
| (\ \) || |   | || |      | |      |  __ (     | |      | |   
| | \   || |   | || |      | |      | (  \ \    | |      | |   
| )  \  || (___) || (____/\| (____/\| )___) )___) (___   | |   
|/    )_)(_______)(_______/(_______/|/ \___/ \_______/   )_(   
    )" << "\033[0m" << endl;

	cout << "\033[38;2;255;255;255m" << "Made by @trinyxt." << endl;
}

void mainLoop() {
	while (true)
	{
		uintptr_t newDataModel = findDatamodel(globals::baseAddress, false);
		if (globals::datamodel != newDataModel)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1500));
			globals::datamodel = newDataModel;
			globals::placeId = findPlaceId(globals::datamodel);
			globals::GameId = findGameId(globals::datamodel);
			globals::JobId = findJobId(globals::datamodel);
			std::cout << "\n\n\nPlace ID: " << globals::placeId << std::endl;
			globals::workspace = findWorkspace(globals::datamodel, true);
			globals::players = findPlayerService(globals::datamodel, true);
			globals::localplayer = findLocalPlayer(globals::players, true);

			globals::OriginalGravityValue = read<float>(globals::prim + offsets::PrimitiveGravity);
			gui::originalWalkSpeed = read<float>(globals::humanoid + offsets::WalkSpeed);
			gui::originalJumpPower = read<float>(globals::humanoid + offsets::JumpPower);

		}

		if (findLocalPlayer(globals::players, false) == globals::localplayer && globals::placeId != 0)
		{
			globals::character = findChar(globals::localplayer, false);
			globals::humanoid = findHumanoid(globals::character, false);
			globals::hrp = findfirstchildbyname(globals::character, "HumanoidRootPart");
			globals::prim = read<uintptr_t>(globals::hrp + offsets::Primitive);
		}

		SetWalkSpeed();
		SetJumpPower();
		SetNoclip();
		SetGravity();

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

int InitialiseDriverAndSome()
{
	globals::hProc = CreateFileA(
		"\\\\.\\Drivah",
		GENERIC_READ,
		0,
		nullptr,
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		nullptr
	);

	if (globals::hProc == INVALID_HANDLE_VALUE) {
		std::cerr << "[-] Driver Wasn't found: Make sure you loaded it! " << "\n" << "\n";
		system("pause");
		return 1;
	}

	globals::PID = GetProcId(roblox);
	if (globals::PID == 0) {
		cout << "[-] Make sure Roblox is opened!" << endl;
		system("pause");
		return 1;
	}

	globals::baseAddress = GetModuleBaseAddy(globals::PID, roblox);

	if (globals::baseAddress == 0) {
		cerr << "[-] Failed to get module base address." << endl;
		system("pause");
		CloseHandle(globals::hProc);
		return -1;
	}

	cout << "Module base address: " << hex << globals::baseAddress << endl;


	if (attach_to_process(globals::PID) == true)
	{
		cout << "[+] Attached to roblox's process successfully." << endl << endl;
	}
	else{
		cout << "[-] Couldn't attach to roblox's process. Make sure to tell trinyxt that he's a dumbass";
		system("pause");
	}
}

void findAddresses()
{
	globals::datamodel = findDatamodel(globals::baseAddress, true);

	globals::placeId = findPlaceId(globals::datamodel);
	globals::GameId = findGameId(globals::datamodel);
	globals::JobId = findJobId(globals::datamodel);
	cout << "\n\n\n[!] Place ID: " << globals::placeId << std::endl;
	globals::workspace = findWorkspace(globals::datamodel, true);

	globals::players = findPlayerService(globals::datamodel, true);
	globals::localplayer = findLocalPlayer(globals::players, true);
	globals::character = findChar(globals::localplayer, false);
	globals::humanoid = findHumanoid(globals::character, false);
	globals::hrp = findfirstchildbyname(globals::character, "HumanoidRootPart");
	globals::prim = read<uintptr_t>(globals::hrp + offsets::Primitive);

	gui::originalWalkSpeed = read<float>(globals::humanoid + offsets::WalkSpeed);
	gui::originalJumpPower = read<float>(globals::humanoid + offsets::JumpPower);
	globals::OriginalGravityValue = read<float>(globals::prim + offsets::PrimitiveGravity);
}

int main() {
	printLogoandcredts();
	cout << endl;

	if(InitialiseDriverAndSome() == 1)
		return 1;

	findAddresses();

	std::thread flyThread(mainLoop);

	return makeWindow((int)globals::PID, nullptr);
}