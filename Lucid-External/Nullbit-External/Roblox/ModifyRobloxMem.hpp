#pragma once

#include "../Driver/driver.hpp"
#include "offsets.hpp"
#include "Globals.h"
#include "../FindAdrress.hpp"


void SetWalkSpeed() {
    if (!globals::humanoid) return;

    float current = read<float>(globals::humanoid + offsets::WalkSpeed);

    if (gui::Walkspeed && current != gui::WalkspeedValue) {
        write<float>(globals::humanoid + offsets::WalkSpeed, gui::WalkspeedValue);
        write<float>(globals::humanoid + offsets::WalkSpeedCheck, gui::WalkspeedValue);
    }
    else if (!gui::Walkspeed && current != gui::originalWalkSpeed) {
        write<float>(globals::humanoid + offsets::WalkSpeed, gui::originalWalkSpeed);
        write<float>(globals::humanoid + offsets::WalkSpeedCheck, gui::originalWalkSpeed);
    }
}

void SetJumpPower() {
    if (!globals::humanoid) return;

    float current = read<float>(globals::humanoid + offsets::JumpPower);

    if (gui::JumpPower && current != gui::JumpPowerValue)
        write<float>(globals::humanoid + offsets::JumpPower, gui::JumpPowerValue);

    else if (!gui::JumpPower && current != gui::originalJumpPower)
        write<float>(globals::humanoid + offsets::JumpPower, gui::originalJumpPower);
}

bool GravChanged = false;

void SetGravity() {
    if (!globals::prim) return;

    float current = read<float>(globals::prim + offsets::PrimitiveGravity);

    if (gui::Gravity && current != gui::GravityValue) {
        write<float>(globals::prim + offsets::PrimitiveGravity, gui::GravityValue);
        GravChanged = true;
    }
    else if (GravChanged && !gui::Gravity && current != globals::OriginalGravityValue) {
        write<float>(globals::prim + offsets::PrimitiveGravity, globals::OriginalGravityValue);
        GravChanged = false;
    }
}