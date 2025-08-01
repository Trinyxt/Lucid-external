#pragma once
#include <cstdint>
#include <string>

namespace imgui {
	inline HWND hwnd = NULL;
}

namespace gui {
	// chatgpt ahh sorting

	// Original Defaults //
	inline float originalWalkSpeed = 16.0f;
	inline float originalJumpPower = 50.0f;

	// Feature Toggles //
	inline bool showImGuiWindow = true;
	inline bool ShowConsole = false;

	// ESP Toggles //
	inline bool tracers = false;
	inline bool Expirimental = false;

	inline bool ESP = false;
	inline bool TeamESP = true;
	inline bool EnemyESP = true;
	inline bool ScreenBounds = false;
	inline bool HealthCheck = false;

	inline float Thickness = 3.0f;
	inline bool NameEsp = false;
	
	inline bool SkeletonESP = false;

	// Aimbot
	inline bool Aimbot = false;

	// Movement Toggles //
	inline bool JumpPower = false;
	inline bool Walkspeed = false;
	inline bool Gravity = false;
	inline bool Fly = false;
	inline bool noclip;

	// Movement Values //
	inline float JumpPowerValue = 50.0f;
	inline float WalkspeedValue = 16.0f;
	inline float GravityValue = 196.2f;
	inline float FlyValue = 95.0f;
}

namespace globals
{
	//Main
	inline DWORD PID = 0;
	inline HANDLE hProc = nullptr;
	inline uintptr_t baseAddress = 0;

	// World
	inline uintptr_t datamodel = 0;
	inline uintptr_t workspace = 0;
	inline uintptr_t placeId = 0;
	inline uintptr_t GameId = 0;
	inline uintptr_t JobId = 0;

	// Player
	inline uintptr_t players = 0;
	inline std::string PlrName = "None" ;
	inline uintptr_t localplayer = 0;
	inline uintptr_t character = 0;
	inline uintptr_t humanoid = 0;
	inline uintptr_t hrp = 0;
	inline uintptr_t prim = 0;
	inline float OriginalGravityValue = 0;

	//other
	inline uintptr_t camera = 0;


}

namespace aimbot {
	inline bool MouseCamera = false;
	inline std::string LockKey = "Q";

	inline uintptr_t currentTarget = 0;
	inline bool RenderFOV = false;
	inline float AimbotSpeed = 2.6f;
	inline float FOVSize = 120.0f;
	inline float smoothness = 2.5f;
	inline const char* targetPartName = "None";

	// checks
	inline bool WallCheck = false;
	inline bool DeadCheck = false;
	inline bool TeamCheck = false;
	inline bool DistanceCheck = false;
	inline float DistanceCheckValue = 1000.0f;
}