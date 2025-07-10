#pragma once
#include <Windows.h>
#include <iostream> 
#include <vector>
#include <string>
#include "offsets.hpp"
#include "MemUtils.hpp"

std::string getname(HANDLE hProc, uintptr_t addy) {
    uintptr_t nameptr = read<uintptr_t>(hProc, addy + offsets::Name); // addy + 0x18

    std::string name; // empty name var

    if (read<int>(hProc, nameptr + 0x10) >= 16) { // 0x10 returns size of string, if its bigger than 16 means its too big to read as a string directly
        uintptr_t nameptr2 = read<uintptr_t>(hProc, nameptr); // read nameptr again
        while (read<char>(hProc, nameptr2) != '\0') { // read each character inside the nameptr2 until the character is = '\0' which is empty
            name += read<char>(hProc, nameptr2); // add the character to name since its not 
            nameptr2++; // increase the index to the next one so that the next char gets read
        }
    }
    else {
        name = read<std::string>(hProc, nameptr); // not too big so we can read directly
    }

    return name; // return the final name
}

std::string getclassName(HANDLE hProc, uintptr_t addy) {
    uintptr_t classdesc = read<uintptr_t>(hProc, addy + offsets::ClassDescriptor); // 0x18
    uintptr_t classname = read<uintptr_t>(hProc, classdesc + offsets::ClassDescriptorToName); // 0x8
    std::string name; // empty name var

    if (!classname) { // check if the classname ptr exists
        return "Unknown";
    }

    if (read<int>(hProc, classname + 0x10) >= 16) { // 0x10 returns size of string, if its bigger than 16 means its too big to read as a string directly
        uintptr_t nameptr = read<uintptr_t>(hProc, classname); // read nameptr again
        while (read<char>(hProc, nameptr) != '\0') { // read each character inside the nameptr2 until the character is = '\0' which is empty
            name += read<char>(hProc, nameptr); // add the character to name since its not 
            nameptr++; // increase the index to the next one so that the next char gets read
        }
    }
    else {
        name = read<std::string>(hProc, classname); // not too big so we can read directly
    }

    return name;
}

std::vector<uintptr_t> getchildren(HANDLE hProc, uintptr_t addy) {
    std::vector<uintptr_t> children; // vector map of uintptrs that will store all the children

    uintptr_t childrenptr = read<uintptr_t>(hProc, addy + offsets::Children); // 0x80
    uintptr_t childrenend = read<uintptr_t>(hProc, childrenptr + offsets::ChildrenEnd); // 0x8, same as the size offset

    for (auto child = read<uintptr_t>(hProc, childrenptr); child < childrenend; child += 0x10) { // for each child in children, while the child's ptr is less than the children end it will read then add 0x10 for the next child in the loop
        children.push_back(read<uintptr_t>(hProc, child)); // push back the ptr of the child
    }

    return children; // return the list of children ptrs
}

uintptr_t findfirstchildbyname(HANDLE hProc, uintptr_t addy, std::string name) {
    for (auto child : getchildren(hProc, addy)) { // for each item "child" in the list of children of addy (we will reuse our getchildren function here)
        if (getname(hProc, child) == name) { // we check if the name of the child is the same as the name we provided
            return child; // if the check passes it returns the child
        }
    }

    return 0; // else it returns 0x0 means no child with that name found
}

uintptr_t findfirstchildbyclass(HANDLE hProc, uintptr_t addy, std::string name) {
    // almost the same logic as findfirstchildbyname but here we are getting the class name and not the childs name
    for (auto child : getchildren(hProc, addy)) {
        if (getclassName(hProc, child) == name) {
            return child;
        }
    }
    return 0;
}

uintptr_t findDatamodel(HANDLE handle,uintptr_t baseaddy, bool shouldprint)
{
    uintptr_t FakeDMlPtr = read<uintptr_t>(handle, baseaddy + offsets::FakeDataModelPointer);
    if (FakeDMlPtr == 0) { // if the datamodel is 0x0 means it was not found
        std::cout << "FakeDMlPtr not found!" << std::endl;
		CloseHandle(handle);
        return 0; // return 0x0
    }
    uintptr_t dm = read<uintptr_t>(handle, FakeDMlPtr + offsets::FakeDataModelToDataModel);
    if (dm == 0) {
        std::cout << "Datamodel not found!" << std::endl;
        CloseHandle(handle);
        return 0; // return 0x0
    }


    if (shouldprint)
        std::cout << "DataModel found at: " << std::hex << dm << std::endl; // print the address of the datamodel
    return dm;
}

uintptr_t findPlayerService(HANDLE handle, uintptr_t dm, bool shouldprint)
{
    uintptr_t PlayerService = findfirstchildbyclass(handle, dm, "Players");
    if (PlayerService == 0) { // if the player service is 0x0 means it was not found
        if (shouldprint)
            std::cout << "Player service not found!" << std::endl;
        CloseHandle(handle);
        return 0; // return 0x0
    }
    if (shouldprint)
        std::cout << "Player service found at: " << std::hex << PlayerService << std::endl; // print the address of the player service
    return PlayerService; // return the address of the player service
}

uintptr_t findLocalPlayer(HANDLE handle, uintptr_t plrservice, bool shouldprint)
{
    uintptr_t localplayer = read<uintptr_t>(handle, plrservice + offsets::LocalPlayer); // 0x28
    if (localplayer == 0) { // if the local player is 0x0 means it was not found
        if (shouldprint) 
            std::cout << "Local player not found!" << std::endl;
        return 0; // return 0x0
    }
    if (shouldprint) 
        std::cout << "Local player found at: " << std::hex << localplayer << std::endl; // print the address of the local player
	return localplayer; // return the address of the local player
}

uintptr_t findChar(HANDLE handle, uintptr_t lcalplr, bool shouldprint)
{
    uintptr_t Char = read<uintptr_t>(handle, lcalplr + offsets::ModelInstance); // 0x28
    if (Char == 0) { // if the local player is 0x0 means it was not found
        if (shouldprint)
            std::cout << "Local player not found!" << std::endl;
        return 0; // return 0x0
    }
    if (shouldprint)
        std::cout << "Character found at: " << std::hex << Char << std::endl; // print the address of the local player
    return Char; // return the address of the local player
}

uintptr_t findHumanoid(HANDLE handle, uintptr_t Char, bool shouldprint)
{
	uintptr_t Humanoid = findfirstchildbyname(handle, Char, "Humanoid");
    if (Humanoid == 0) { // if the humanoid is 0x0 means it was not found
        if (shouldprint)
            std::cout << "Humanoid not found!" << std::endl;
        return 0; // return 0x0
    }
    if (shouldprint)
        std::cout << "Humanoid found at: " << std::hex << Humanoid << std::endl; // print the address of the humanoid
    return Humanoid; // return the address of the humanoid
}

