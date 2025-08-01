#pragma once

#include "../Driver/driver.hpp"
#include "offsets.hpp"
#include "Globals.h"
#include "../FindAdrress.hpp"

void TranslateBy(float speed)
{
    if (!globals::humanoid || !globals::hrp)
        return;

    Vector3 moveDirection = read<Vector3>(globals::humanoid + offsets::MoveDirection);
    std::cout << "MoveDir: " << moveDirection.x << ", " << moveDirection.y << ", " << moveDirection.z << std::endl;


    // Step 2: Normalize and scale the direction
    float length = sqrtf(moveDirection.x * moveDirection.x + moveDirection.y * moveDirection.y + moveDirection.z * moveDirection.z);
    if (length == 0) return;

    Vector3 direction = {
        moveDirection.x / length * speed,
        moveDirection.y / length * speed,
        moveDirection.z / length * speed
    };

    // Step 3: Read current position
    Vector3 currentPos = read<Vector3>(globals::hrp + offsets::Position);

    // Step 4: Add offset
    Vector3 newPos = {
        currentPos.x + direction.x,
        currentPos.y + direction.y,
        currentPos.z + direction.z
    };

    // Step 5: Write new position
    write<Vector3>(globals::hrp + offsets::Position, newPos);
}

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