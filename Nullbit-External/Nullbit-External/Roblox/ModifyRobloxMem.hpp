#pragma once

#include "../Driver/driver.hpp"
#include "offsets.hpp"
#include "Globals.h"
#include "../FindAdrress.hpp"

using namespace std;

void SetJumpPower() {
	
	if (gui::JumpPower && read<float>( globals::humanoid + offsets::JumpPower) != gui::JumpPowerValue)
	{
		write<float>( globals::humanoid + offsets::JumpPower, gui::JumpPowerValue);
	}
	else if (!gui::JumpPower && read<float>( globals::humanoid + offsets::JumpPower) != gui::originalJumpPower)
	{
		write<float>( globals::humanoid + offsets::JumpPower, gui::originalJumpPower);
	}
}

void SetWalkSpeed()
{
	if (gui::Walkspeed && read<float>( globals::humanoid + offsets::WalkSpeed) != gui::WalkspeedValue)
	{
		write<float>( globals::humanoid + offsets::WalkSpeed, gui::WalkspeedValue);
		write<float>( globals::humanoid + offsets::WalkSpeedCheck, gui::WalkspeedValue);
	}
	else if (!gui::Walkspeed && read<float>( globals::humanoid + offsets::WalkSpeed) != gui::originalWalkSpeed)
	{
		write<float>( globals::humanoid + offsets::WalkSpeed, gui::originalWalkSpeed);
		write<float>( globals::humanoid + offsets::WalkSpeedCheck, gui::originalWalkSpeed);
	}
}

bool GravChanged = false;

void SetGravity() {
	if (gui::Gravity && read<float>(globals::prim + offsets::PrimitiveGravity) != gui::GravityValue)
	{
		if (GravChanged)
			GravChanged = false;
		write<float>(globals::prim + offsets::PrimitiveGravity, gui::GravityValue);
	}
	else if (!GravChanged && (!gui::Gravity && read<float>(globals::prim + offsets::PrimitiveGravity) != 196.2f))
	{
		write<float>(globals::prim + offsets::PrimitiveGravity, globals::OriginalGravityValue);
		GravChanged = true;
	}
}

void SetNoclip() {
	if (gui::noclip) {
		for (auto child : getchildren(globals::character)) {
			//if (getclassName(child) == "MeshPart") {
				cout << child << endl;
				auto primitive = read<uintptr_t>(child + offsets::Primitive);
				cout << primitive << endl;
				write<bool>(primitive + offsets::CanCollide, false);
				cout << "Noclip: " << read<bool>(primitive + offsets::CanCollide) << endl;
			//}
		}
	}
	else { 
		for (auto child : getchildren(globals::character)) {
			//if (getclassName(child) == "MeshPart") {
				auto primitive = read<uintptr_t>(child + offsets::Primitive);
				write<bool>(primitive + offsets::CanCollide, true);
			//}
		}
	}
}
