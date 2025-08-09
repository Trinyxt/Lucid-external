#pragma once
#include <Windows.h>
#include <iostream> 
#include <vector>
#include <string>
#include "offsets.hpp"
#include "Driver/driver.hpp"
#include "Vendor/math.hpp"
#include "imgui.h"

std::string getname( uintptr_t addy) {
    uintptr_t nameptr = read<uintptr_t>( addy + offsets::Name); // addy + 0x18

    std::string name; // empty name var

    if (read<int>( nameptr + 0x10) >= 16) { // 0x10 returns size of string, if its bigger than 16 means its too big to read as a string directly
        uintptr_t nameptr2 = read<uintptr_t>( nameptr); // read nameptr again
        while (read<char>( nameptr2) != '\0') { // read each character inside the nameptr2 until the character is = '\0' which is empty
            name += read<char>( nameptr2); // add the character to name since its not 
            nameptr2++; // increase the index to the next one so that the next char gets read
        }
    }
    else {
        name = read<std::string>( nameptr); // not too big so we can read directly
    }

    return name; // return the final name
}

std::string getclassName( uintptr_t addy) {
    uintptr_t classdesc = read<uintptr_t>( addy + offsets::ClassDescriptor); // 0x18
    uintptr_t classname = read<uintptr_t>( classdesc + offsets::ClassDescriptorToClassName); // 0x8
    std::string name; // empty name var

    if (!classname) { // check if the classname ptr exists
        return "Unknown";
    }

    if (read<int>( classname + 0x10) >= 16) { // 0x10 returns size of string, if its bigger than 16 means its too big to read as a string directly
        uintptr_t nameptr = read<uintptr_t>( classname); // read nameptr again
        while (read<char>( nameptr) != '\0') { // read each character inside the nameptr2 until the character is = '\0' which is empty
            name += read<char>( nameptr); // add the character to name since its not 
            nameptr++; // increase the index to the next one so that the next char gets read
        }
    }
    else {
        name = read<std::string>( classname); // not too big so we can read directly
    }

    return name;
}

std::vector<uintptr_t> getchildren( uintptr_t addy) {
    std::vector<uintptr_t> children; // vector map of uintptrs that will store all the children

    uintptr_t childrenptr = read<uintptr_t>( addy + offsets::Children); // 0x80
    uintptr_t childrenend = read<uintptr_t>( childrenptr + offsets::ChildrenEnd); // 0x8, same as the size offset

    for (auto child = read<uintptr_t>( childrenptr); child < childrenend; child += 0x10) { // for each child in children, while the child's ptr is less than the children end it will read then add 0x10 for the next child in the loop
        children.push_back(read<uintptr_t>( child)); // push back the ptr of the child
    }

    return children; // return the list of children ptrs
}

uintptr_t findfirstchildbyname( uintptr_t addy, std::string name) {
    for (auto child : getchildren(addy)) { // for each item "child" in the list of children of addy (we will reuse our getchildren function here)
        if (getname(child) == name) { // we check if the name of the child is the same as the name we provided
            return child; // if the check passes it returns the child
        }
    }

    return 0; // else it returns 0x0 means no child with that name found
}

uintptr_t findfirstchildbyclass( uintptr_t addy, std::string name) {
    // almost the same logic as findfirstchildbyname but here we are getting the class name and not the childs name
    for (auto child : getchildren(addy)) {
        if (getclassName(child) == name) {
            return child;
        }
    }
    return 0;
}

uintptr_t findDatamodel(uintptr_t baseaddy, bool shouldprint)
{
    uintptr_t FakeDMPtr = read<uintptr_t>(baseaddy + offsets::FakeDataModelPointer);
    
    if (FakeDMPtr == 0) { 
        std::cout << "FakeDMlPtr not found!" << std::endl;
		CloseHandle(globals::hProc);
        ExitProcess(0);
    }
    uintptr_t dm = read<uintptr_t>(FakeDMPtr + offsets::FakeDataModelToDataModel);
    
    if (dm == 0) {
        std::cout << "Datamodel not found!" << std::endl;
        CloseHandle(globals::hProc);
        return 0; // return 0x0
    }


    if (shouldprint)
        std::cout << "[!] DataModel found at: " << std::hex << dm << std::endl; // print the address of the datamodel
    return dm;
}

uintptr_t findPlaceId( uintptr_t datamodel)
{
	return 
        read<uintptr_t>(datamodel + offsets::PlaceId);

}

uintptr_t findGameId(uintptr_t datamodel)
{
    return
        read<uintptr_t>(datamodel + offsets::GameId);

}

uintptr_t findJobId(uintptr_t datamodel)
{
    return
        read<uintptr_t>(datamodel + offsets::JobId);
}

uintptr_t findPlayerService( uintptr_t dm, bool shouldprint)
{
    uintptr_t PlayerService = findfirstchildbyclass(dm, "Players");
    if (PlayerService == 0) { // if the player service is 0x0 means it was not found
        if (shouldprint)
            std::cout << "[-] Player service not found!" << std::endl;
        CloseHandle(globals::hProc);
        return 0; // return 0x0
    }
    if (shouldprint)
        std::cout << "[!] Player service found at: " << std::hex << PlayerService << std::endl; // print the address of the player service
    return PlayerService; // return the address of the player service
}

uintptr_t findLocalPlayer( uintptr_t plrservice, bool shouldprint)
{
    uintptr_t localplayer = read<uintptr_t>( plrservice + offsets::LocalPlayer); // 0x28
    if (localplayer == 0) { // if the local player is 0x0 means it was not found
        if (shouldprint) 
            std::cout << "[!] Local player not found!" << std::endl;
        return 0; // return 0x0
    }
    if (shouldprint) 
        std::cout << "[!] Local player found at: " << std::hex << localplayer << std::endl; // print the address of the local player
	return localplayer; // return the address of the local player
}

uintptr_t findChar( uintptr_t lcalplr, bool shouldprint)
{
    uintptr_t Char = read<uintptr_t>( lcalplr + offsets::ModelInstance); // 0x28
    if (Char == 0) { // if the local player is 0x0 means it was not found
        if (shouldprint)
            std::cout << "Character not found!" << std::endl;
        return 0; // return 0x0
    }
    if (shouldprint)
        std::cout << "[!] Character found at: " << std::hex << Char << std::endl; // print the address of the local player
    return Char; // return the address of the local player
}

std::vector<uintptr_t> findAllPlayers( uintptr_t plrservice, bool shouldprint)
{
    std::vector<uintptr_t> children = getchildren(plrservice);

    if (shouldprint)
        std::cout << "[*] Found " << children.size() << " children." << std::endl;

    for (size_t i = 0; i < children.size(); ++i) {
        uintptr_t child = children[i];
        if (shouldprint)
            std::cout << "    > Child " << i << " address: 0x" << std::hex << child << std::dec << std::endl;
    }

    // Return the vector holding all children addresses
    return children;
}

uintptr_t findHumanoid( uintptr_t Char, bool shouldprint)
{
	uintptr_t Humanoid = findfirstchildbyname(Char, "Humanoid");
    if (Humanoid == 0) { // if the humanoid is 0x0 means it was not found
        if (shouldprint)
            std::cout << "Humanoid not found!" << std::endl;
        return 0; // return 0x0
    }
    if (shouldprint)
        std::cout << "[!] Humanoid found at: " << std::hex << Humanoid << std::endl; // print the address of the humanoid
    return Humanoid; // return the address of the humanoid
}

uintptr_t findWorkspace( uintptr_t dm, bool shouldprint)
{
    uintptr_t WorkSpace = findfirstchildbyname(dm, "Workspace");
    if (WorkSpace == 0) {
        if (shouldprint)
            std::cout << "WorkSpace not found!" << std::endl;
        //CloseHandle(globals::hProc);
        return 0; // return 0x0
    }
    if (shouldprint)
        std::cout << "[!] WorkSpace found at: " << std::hex << WorkSpace << std::endl; // print the address of the player service
    return WorkSpace; // return the address of the player service
}

std::string GetPlayerName()
{
    std::string plrName = getname(globals::localplayer);
    globals::PlrName = plrName;
    return plrName;
}

Matrix4 GetViewMatrix() {
    uintptr_t base = globals::baseAddress; // our stored base address
    uintptr_t vengineptr = read<uintptr_t>(base + offsets::VisualEnginePointer); // adding the offset to base to get the visual engine ptr
    Matrix4 viewmatrix = read<Matrix4>(vengineptr + offsets::viewmatrix); // this is the viewmatrix

    return viewmatrix;
}

int GetPlayerHealth( uintptr_t player) {
    uintptr_t character = findChar(player, true);
    uintptr_t humanoid = findHumanoid(character, true);

    float health = read<float>( humanoid + offsets::Health);

    return health;
}

int GetPlayerTeam(uintptr_t plr)
{
	return read<int>(plr + offsets::Team);
}

Vector3 GetPlayerPosition(uintptr_t player) {
    if (!player) return { 0, 0, 0 };

    uintptr_t character = read<uintptr_t>(player + offsets::ModelInstance);
    if (!character) return { 0, 0, 0 };

    uintptr_t hrp = findfirstchildbyname(character, aimbot::targetPartName);
    if (!hrp) return { 0, 0, 0 };

    uintptr_t prim = read<uintptr_t>(hrp + offsets::Primitive);
    if (!prim) return { 0, 0, 0 };

    return read<Vector3>(prim + offsets::Position);
}

bool IsCharR15(uintptr_t character)
{

    if (findfirstchildbyname(character, "UpperTorso") != 0)
		return true;
    if (findfirstchildbyname(character, "LowerTorso") != 0)
        return true;

    return false;
}

bool ListExplorer(uintptr_t address) {
    const std::string& name = getname(address);
    const std::string& classname = getclassName(address);

    std::string label = name + " [" + classname + "]";
    std::string itemname = label + "##" + std::to_string(address);

    if (ImGui::TreeNode(itemname.c_str())) {
        const auto& children = getchildren(address);
        for (const auto& child : children) {
            ListExplorer(child);
        }
        ImGui::TreePop();
    }

    return true;
}