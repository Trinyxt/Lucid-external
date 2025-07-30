#pragma once
#include <d3d11.h>
#include <tchar.h>
#include "Vendor/math.hpp"
#include "Roblox/Globals.h"
#include <windows.h>
#include "Driver/driver.hpp"
#include "FindAdrress.hpp"
#include <unordered_map>
#include <algorithm>
#include <TlHelp32.h>

bool aimTgl;

bool IsLockKeyPressed() {
    if (aimbot::LockKey == "Mouse Side Button Down")
        return (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) != 0;
    if (aimbot::LockKey == "Right Mouse Button")
        return (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
    if (aimbot::LockKey == "Q")
        return (GetAsyncKeyState('Q') & 0x8000) != 0;
    if (aimbot::LockKey == "E")
        return (GetAsyncKeyState('E') & 0x8000) != 0;
    if (aimbot::LockKey == "Z")
        return (GetAsyncKeyState('Z') & 0x8000) != 0;
    if (aimbot::LockKey == "X")
        return (GetAsyncKeyState('X') & 0x8000) != 0;

    return false;
}


void MakeAimbot() {
    static uintptr_t currentTarget = 0;

    aimTgl = IsLockKeyPressed();
        
    if (!aimTgl || !gui::Aimbot || gui::showImGuiWindow) {
        currentTarget = 0; // Reset target when aim toggle is off or menu is open
        return;
    }

    RECT clientRect;
    if (!GetClientRect(imgui::hwnd, &clientRect)) return;

    ScreenSize screen;
    screen.Width = clientRect.right - clientRect.left;
    screen.Height = clientRect.bottom - clientRect.top;

    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) return;
    if (!ScreenToClient(imgui::hwnd, &cursorPos)) return;

    Vector2 mousepos = { (float)cursorPos.x, (float)cursorPos.y };
    Vector2 dimensions = { (float)screen.Width, (float)screen.Height };
    Matrix4 viewmatrix = GetViewMatrix();

    std::string PlrName = getname(globals::localplayer);
    int PlrHealth = GetPlayerHealth(globals::localplayer);
    int PlrTeam = GetPlayerTeam(globals::localplayer);

    Vector2 targetScreenPos = { -1, -1 };

    auto isValidTarget = [&](uintptr_t player) -> bool {
        if (getname(player) == PlrName) return false;
        if (aimbot::DeadCheck && GetPlayerHealth(player) <= 0) return false;
        if (aimbot::TeamCheck && GetPlayerTeam(player) == PlrTeam) return false;

        uintptr_t character = read<uintptr_t>(player + offsets::ModelInstance);
        uintptr_t hrp = findfirstchildbyname(character, aimbot::targetPartName);
        uintptr_t prim = read<uintptr_t>(hrp + offsets::Primitive);

        if (!hrp || !prim) return false;

        Vector3 worldPos = read<Vector3>(prim + offsets::Position);
        if (worldPos.x == 0 && worldPos.y == 0 && worldPos.z == 0) return false;

        Vector2 screenPos = WorldToScreen(worldPos, dimensions, viewmatrix);
        if (screenPos.x == -1 || screenPos.y == -1) return false;

        float dist = sqrtf(powf(screenPos.x - mousepos.x, 2) + powf(screenPos.y - mousepos.y, 2));
        if (dist > aimbot::FOVSize) return false;

        targetScreenPos = screenPos;
        return true;
        };

    if (!isValidTarget(currentTarget)) {
        currentTarget = 0;
        float closestDistance = FLT_MAX;

        for (auto& player : getchildren(globals::players)) {
            uintptr_t candidate = player;
            if (!isValidTarget(candidate)) continue;

            float distToCursor = sqrtf(
                powf(targetScreenPos.x - mousepos.x, 2) +
                powf(targetScreenPos.y - mousepos.y, 2));

            if (distToCursor < closestDistance) {
                closestDistance = distToCursor;
                currentTarget = candidate;
            }
        }

        if (currentTarget && isValidTarget(currentTarget) == false) {
            currentTarget = 0;
            return;
        }
    }

    if (!currentTarget) return;

    float dx = targetScreenPos.x - mousepos.x;
    float dy = targetScreenPos.y - mousepos.y;

    if (fabsf(dx) < 1.0f && fabsf(dy) < 1.0f)
        return;

    float aimSpeed = aimbot::smoothness * 2.0f;
    if (aimSpeed < 1.0f)
        aimSpeed = 1.0f;

    dx /= aimSpeed;
    dy /= aimSpeed;

    INPUT input = { 0 };
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = static_cast<LONG>(roundf(dx));
    input.mi.dy = static_cast<LONG>(roundf(dy));
    SendInput(1, &input, sizeof(INPUT));
}