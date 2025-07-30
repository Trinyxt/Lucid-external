#include <Windows.h>
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <atomic>

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

std::atomic<bool> keepRunning = true;

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

void mainLoop() {
	while (keepRunning)
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

		if (globals::placeId != 0)
		{
			uintptr_t newCharacter = findChar(globals::localplayer, false);

			if (newCharacter != globals::character)
			{
				globals::character = newCharacter;

				globals::humanoid = findHumanoid(newCharacter, false);

				uintptr_t hrp = findfirstchildbyname(newCharacter, "HumanoidRootPart");
				globals::hrp = hrp;
				globals::prim = hrp ? read<uintptr_t>(hrp + offsets::Primitive) : 0;
			}
		}

		SetWalkSpeed();
		SetJumpPower();
		SetGravity();

		std::this_thread::sleep_for(std::chrono::milliseconds(500));
	}
}

#include <wininet.h>
#pragma comment(lib, "wininet.lib")

std::string Status() {
	const char* url = "https://pastefy.app/RT3CwXz6/raw";

	HINTERNET hInternet = InternetOpenA("Nullbit", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
	if (!hInternet) {
		std::cerr << "InternetOpenA failed\n";
		return ""; // Return empty string on failure
	}

	HINTERNET hFile = InternetOpenUrlA(hInternet, url, NULL, 0, INTERNET_FLAG_RELOAD, 0);
	if (!hFile) {
		std::cerr << "InternetOpenUrlA failed\n";
		InternetCloseHandle(hInternet);
		return ""; // Return empty string on failure
	}

	char buffer[4096];
	DWORD bytesRead;
	std::string content;

	while (InternetReadFile(hFile, buffer, sizeof(buffer), &bytesRead) && bytesRead > 0) {
		content.append(buffer, bytesRead);
	}

	InternetCloseHandle(hFile);
	InternetCloseHandle(hInternet);

	return content;
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
	}

	globals::PID = GetProcId(roblox);
	if (globals::PID == 0) {
		cout << "[-] Make sure Roblox is opened!" << endl;
		system("pause");
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



int main() {
	string status = Status();
	printLogoandcredts();

	if (status == "down"){
		cout << "\033[91m[!] Nullbit is currently down for maintenance.\033[0m" << endl;
		cout << "Please click anything ." << endl;
		cin.get();
		return 0;
	}

	if (InitialiseDriverAndSome() == 1)
		return 1;

	findAddresses();

	std::thread flyThread(mainLoop);

	::ShowWindow(::GetConsoleWindow(), SW_HIDE);
	makeWindow((int)globals::PID, nullptr);
	
	keepRunning = false; 
	flyThread.join();
	return 0;
}