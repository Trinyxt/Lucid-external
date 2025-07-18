#pragma once
#include <cstdint>

namespace offsets {

    // Visual Engine
    inline constexpr uintptr_t VisualEngine = 0x10;
    inline constexpr uintptr_t VisualEnginePointer = 0x6676BC8;
    inline constexpr uintptr_t VisualEngineToDataModel1 = 0x700;
    inline constexpr uintptr_t VisualEngineToDataModel2 = 0x1C0;

    // Model / DataModel related
    inline constexpr uintptr_t FakeDataModelPointer = 0x6833728;
    inline constexpr uintptr_t FakeDataModelToDataModel = 0x1C0; inline constexpr uintptr_t ModelInstance = 0x340;
    inline constexpr uintptr_t Primitive = 0x178;
    inline constexpr uintptr_t PrimitiveGravity = 0x110;

    // Player related
    inline constexpr uintptr_t LocalPlayer = 0x128;
    inline constexpr uintptr_t WalkSpeed = 0x1DC;
    inline constexpr uintptr_t WalkSpeedCheck = 0x3B8;
    inline constexpr uintptr_t Team = 0x268;
    inline constexpr uintptr_t JumpPower = 0x1B8;
    inline constexpr uintptr_t Health = 0x19C;
    inline constexpr uintptr_t Position = 0x13C;
    inline constexpr uintptr_t PlaceId = 0x1A0;
    inline constexpr uintptr_t GameId = 0x198;
    inline constexpr uintptr_t CanCollide = 0x2FB;
    inline constexpr uintptr_t JobId = 0x140;

    // Children / Hierarchy
    inline constexpr uintptr_t Children = 0x80;
    inline constexpr uintptr_t ChildrenEnd = 0x8;

    // Rendering / View related
    inline constexpr uintptr_t viewmatrix = 0x4B0;
    inline constexpr uintptr_t ViewportSize = 0x300;

    // Object naming / Class info
    inline constexpr uintptr_t Name = 0x78;
    inline constexpr uintptr_t ClassDescriptor = 0x18;
    inline constexpr uintptr_t ClassDescriptorToName = 0x8;

    // Physics related
    inline constexpr uintptr_t Gravity = 0x960;

}
