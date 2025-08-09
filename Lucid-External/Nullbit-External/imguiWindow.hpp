// Dear ImGui: standalone example application for DirectX 11

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "Roblox/Aimbot.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include "Vendor/math.hpp"
#include "Roblox/Globals.h"
#include <windows.h>
#include "Driver/driver.hpp"
#include "FindAdrress.hpp"
#include <unordered_map>
#include <string>
#include <algorithm>
#include <TlHelp32.h>


#define NOMINMAX




ImVec4 GetRainbowColor(float speed = 1.0f, float alpha = 1.0f)
{
    float t = ImGui::GetTime() * speed;
    float r = (sinf(t * 6.28318f) + 1.0f) * 0.5f;
    float g = (sinf(t * 6.28318f + 2.09439f) + 1.0f) * 0.5f;  // +120 degrees phase
    float b = (sinf(t * 6.28318f + 4.18878f) + 1.0f) * 0.5f;  // +240 degrees phase
    return ImVec4(r, g, b, alpha);
}

static ImVec4 espColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
static ImVec4 RainbowespColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

static ImVec4 FOVColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
static ImVec4 FOVRainbowColor = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
static int FOVcolorMode = 0;

static float thickness = 1.0f;
static int colorMode = 0;

void SetClickThrough(bool enable) {
    HWND hwnd = imgui::hwnd;
    LONG exStyle = GetWindowLong(imgui::hwnd, GWL_EXSTYLE);
    if (enable) {
        SetWindowLong(imgui::hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT);
    }
    else {
        SetWindowLong(imgui::hwnd, GWL_EXSTYLE, exStyle & ~WS_EX_TRANSPARENT);
    }
}

Vector2 aimbotTarget = Vector2(-1, -1);
float closestDistance = FLT_MAX;


void DrawESP() {
    ImDrawList* drawlist = ImGui::GetBackgroundDrawList();

    RECT clientRect;
    if (!GetClientRect(imgui::hwnd, &clientRect)) return;

    ScreenSize screen{
        clientRect.right - clientRect.left,
        clientRect.bottom - clientRect.top
    };

    POINT cursorPos;
    if (!GetCursorPos(&cursorPos)) return;
    if (!ScreenToClient(imgui::hwnd, &cursorPos)) return;

    Vector2 mousepos = Vector2{ (float)cursorPos.x, (float)cursorPos.y };
    Vector2 dimensions = Vector2{ (float)screen.Width, (float)screen.Height };

    Matrix4 viewmatrix = GetViewMatrix();
    auto localTeam = GetPlayerTeam(globals::localplayer);

    for (auto player : getchildren(globals::players)) {
        if (player == globals::localplayer) continue;
        if (gui::HealthCheck && GetPlayerHealth(player) <= 0) continue;

        auto playerTeam = GetPlayerTeam(player);
        if ((!gui::TeamESP && playerTeam == localTeam) || (!gui::EnemyESP && playerTeam != localTeam)) continue;

        uintptr_t character = read<uintptr_t>(player + offsets::ModelInstance);
        if (!character) continue;

        uintptr_t hrp = findfirstchildbyname(character, "HumanoidRootPart");
        if (!hrp) continue;

        uintptr_t prim = read<uintptr_t>(hrp + offsets::Primitive);
        if (!prim) continue;

        Vector3 pos = read<Vector3>(prim + offsets::Position);
        if (pos == Vector3{ 0, 0, 0 }) continue;

        Vector2 screenpos = WorldToScreen(pos, dimensions, viewmatrix);
        if (screenpos.x == -1.0f || screenpos.y == -1.0f) continue;

        ImColor lineColor = (colorMode == 0) ?
            ImColor(espColor.x, espColor.y, espColor.z, espColor.w) :
            ImColor((RainbowespColor = GetRainbowColor(1.0f, 1.0f)));

        if (gui::tracers) {
            ImVec2 from = ImVec2(mousepos.x, mousepos.y);
            ImVec2 to = ImVec2(screenpos.x, screenpos.y);
            drawlist->AddLine(from, to, lineColor, gui::Thickness);
        }

        if (gui::ESP || gui::NameEsp) {
            Vector3 headWorld = VectorAdd(pos, Vector3{ 0.f, 2.5f, 0.f });
            Vector3 footWorld = VectorSub(pos, Vector3{ 0.f, 4.0f, 0.f });

            Vector2 head = WorldToScreen(headWorld, dimensions, viewmatrix);
            Vector2 foot = WorldToScreen(footWorld, dimensions, viewmatrix);
            if (head.x == -1 || foot.x == -1) continue;

            if (gui::ESP) {
                float height = abs(head.y - foot.y) * 1.1f;
                float width = height / 2.0f;
                float centerx = (head.x + foot.x) * 0.5f;

                ImVec2 pmin = ImVec2(centerx - width / 1.5f, head.y - (height * 0.05f));
                ImVec2 pmax = ImVec2(centerx + width / 1.5f, foot.y + (height * 0.05f));

                drawlist->AddRect(pmin, pmax, IM_COL32(0, 0, 0, 255), 0.f, 0, gui::Thickness + 2.f);
                drawlist->AddRect(pmin, pmax, lineColor, 0.f, 0, gui::Thickness);
            }

            if (gui::NameEsp) {
                std::string name = getname(player);
                ImVec2 textSize = ImGui::CalcTextSize(name.c_str());
                ImVec2 textPos = ImVec2(head.x - textSize.x / 2.f, head.y - textSize.y - 4.f);

                ImU32 outlineColor = IM_COL32(0, 0, 0, 255);
                for (int dx = -1; dx <= 1; dx++) {
                    for (int dy = -1; dy <= 1; dy++) {
                        if (dx == 0 && dy == 0) continue;
                        drawlist->AddText(ImVec2(textPos.x + dx, textPos.y + dy), outlineColor, name.c_str());
                    }
                }
                drawlist->AddText(textPos, lineColor, name.c_str());
            }
        }

        if (gui::SkeletonESP) {
            bool isR15 = IsCharR15(character);

            struct BoneConnection { const char* from, * to; };
            static const std::vector<BoneConnection> Rig6 = {
                {"Head", "Torso"}, {"Torso", "Left Arm"}, {"Torso", "Right Arm"},
                {"Torso", "Left Leg"}, {"Torso", "Right Leg"}
            };

            static const std::vector<BoneConnection> Rig15 = {
                {"Head", "UpperTorso"}, {"UpperTorso", "LowerTorso"},
                {"UpperTorso", "LeftUpperArm"}, {"UpperTorso", "RightUpperArm"},
                {"LeftUpperArm", "LeftLowerArm"}, {"RightUpperArm", "RightLowerArm"},
                {"LeftLowerArm", "LeftHand"}, {"RightLowerArm", "RightHand"},
                {"LowerTorso", "LeftUpperLeg"}, {"LowerTorso", "RightUpperLeg"},
                {"LeftUpperLeg", "LeftLowerLeg"}, {"RightUpperLeg", "RightLowerLeg"},
                {"LeftLowerLeg", "LeftFoot"}, {"RightLowerLeg", "RightFoot"}
            };

            const auto& rig = isR15 ? Rig15 : Rig6;
            for (const auto& bone : rig) {
                uintptr_t from = findfirstchildbyname(character, bone.from);
                uintptr_t to = findfirstchildbyname(character, bone.to);
                if (!from || !to) continue;

                uintptr_t fprim = read<uintptr_t>(from + offsets::Primitive);
                uintptr_t tprim = read<uintptr_t>(to + offsets::Primitive);
                if (!fprim || !tprim) continue;

                Vector3 fpos = read<Vector3>(fprim + offsets::Position);
                Vector3 tpos = read<Vector3>(tprim + offsets::Position);

                Vector2 fscreen = WorldToScreen(fpos, dimensions, viewmatrix);
                Vector2 tscreen = WorldToScreen(tpos, dimensions, viewmatrix);
                if (fscreen.x == -1 || tscreen.x == -1) continue;

                drawlist->AddLine(ImVec2(fscreen.x, fscreen.y), ImVec2(tscreen.x, tscreen.y), IM_COL32(0, 0, 0, 255), 3.f);
                drawlist->AddLine(ImVec2(fscreen.x, fscreen.y), ImVec2(tscreen.x, tscreen.y), IM_COL32(255, 255, 255, 255));
            }
        }
    }

    if (aimbot::RenderFOV) {
        ImColor lineColor = (FOVcolorMode == 0) ?
            ImColor(FOVColor.x, FOVColor.y, FOVColor.z, FOVColor.w) :
            ImColor((FOVRainbowColor = GetRainbowColor(1.0f, 1.0f)));

        drawlist->AddCircle(ImVec2(mousepos.x, mousepos.y), aimbot::FOVSize, lineColor, 1024, 5.0f);
    }
}

void styling()
{
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);
    style.FontScaleDpi = main_scale;

    ImVec4* colors = style.Colors;

    // Backgrounds - Deeper blacks with soft alpha
    colors[ImGuiCol_WindowBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.95f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
    colors[ImGuiCol_PopupBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.98f);

    // Borders - light contrast gray
    colors[ImGuiCol_Border] = ImVec4(0.40f, 0.40f, 0.40f, 0.25f);

    // Title Bar
    colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.80f);

    // Frames
    colors[ImGuiCol_FrameBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.15f, 0.15f, 0.15f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);

    // Buttons
    colors[ImGuiCol_Button] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

    // Checkmark - bright
    colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);

    // Sliders
    colors[ImGuiCol_SliderGrab] = ImVec4(0.90f, 0.90f, 0.90f, 0.60f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);

    // Tabs
    colors[ImGuiCol_Tab] = ImVec4(0.07f, 0.07f, 0.07f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
    colors[ImGuiCol_TabActive] = ImVec4(0.12f, 0.12f, 0.12f, 1.00f);
    colors[ImGuiCol_TabUnfocused] = ImVec4(0.05f, 0.05f, 0.05f, 0.70f);

    // Text
    colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f); // Pure white
    colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);   // Mid-gray

    // Separators
    colors[ImGuiCol_Separator] = ImVec4(0.50f, 0.50f, 0.50f, 0.45f);
    colors[ImGuiCol_SeparatorHovered] = ImVec4(0.75f, 0.75f, 0.75f, 0.65f);
    colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.90f);

    // Scrollbars
    colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.60f);
    colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.20f, 0.20f, 0.20f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.30f, 0.80f);
    colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.35f, 0.35f, 0.35f, 1.00f);

    // Rounding - square UI
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.FrameRounding = 0.0f;
    style.PopupRounding = 0.0f;
    style.ScrollbarRounding = 0.0f;
    style.GrabRounding = 0.0f;
    style.TabRounding = 0.0f;

    // Borders
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;

    // Spacing
    style.WindowPadding = ImVec2(12, 12);
    style.FramePadding = ImVec2(8, 6);
    style.ItemSpacing = ImVec2(8, 6);
    style.ItemInnerSpacing = ImVec2(4, 4);
}


static const char* SelectedBind = 0;

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


HWND GetHWNDFromPID(DWORD pid) {
    HWND hWnd = nullptr;
    do {
        hWnd = FindWindowEx(nullptr, hWnd, nullptr, nullptr);
        DWORD wndPid = 0;
        GetWindowThreadProcessId(hWnd, &wndPid);
        if (wndPid == pid && IsWindowVisible(hWnd))
            return hWnd;
    } while (hWnd != nullptr);
    return nullptr;
}


// Main code
int makeWindow(int, char**)
{
    ImGui_ImplWin32_EnableDpiAwareness();
    float main_scale = ImGui_ImplWin32_GetDpiScaleForMonitor(::MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY));

    // Create application window
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"ImGuiClass", nullptr };
    ::RegisterClassExW(&wc);
    
    RECT client;
    POINT topLeft = { 0, 0 };

    HWND robloxWindow = GetHWNDFromPID(globals::PID);
    if (!robloxWindow) {
        std::cerr << "Failed to find Roblox window!" << std::endl;
        return 1;
    }

    GetClientRect(robloxWindow, &client);
    ClientToScreen(robloxWindow, &topLeft);

    int width = client.right - client.left;
    int height = client.bottom - client.top;



    imgui::hwnd = ::CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW,
        wc.lpszClassName,
        L"Lucid",
        WS_POPUP,
        topLeft.x, topLeft.y,
        width, height,
        nullptr, nullptr, wc.hInstance, nullptr);
    
    ::SetLayeredWindowAttributes(imgui::hwnd, RGB(0, 0, 0), 0, LWA_COLORKEY);

    if (!CreateDeviceD3D(imgui::hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    // Show the window
    ::ShowWindow(imgui::hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(imgui::hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeuib.ttf", 15.0f); // Segoe UI Bold

    styling();


    // Setup Platform/Renderer backends
    ImGui_ImplWin32_Init(imgui::hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Main loop
    bool done = false;
    while (!done)
    {
        if (GetAsyncKeyState(VK_INSERT) & 1) // `& 1` triggers on key press (not hold)
        {
            gui::showImGuiWindow = !gui::showImGuiWindow;
        }

        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        // Handle window being minimized or screen locked
        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        // Handle window resize (we don't resize directly in the WM_SIZE handler)
        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        // Start the Dear ImGui frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();

        ImGui::NewFrame();

        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Once, ImVec2(0.5f, 0.5f));

        RECT client;
        POINT topLeft = { 0, 0 };
        GetClientRect(robloxWindow, &client);
        ClientToScreen(robloxWindow, &topLeft);

        int width = client.right - client.left;
        int height = client.bottom - client.top;

        SetWindowPos(imgui::hwnd, HWND_TOPMOST,
            topLeft.x, topLeft.y,
            width, height,
            SWP_NOACTIVATE | SWP_NOZORDER);



        ImGui::SetNextWindowSize(ImVec2(550, 610), ImGuiCond_Once);

        static bool showWatermark = true;

        if (gui::showImGuiWindow)
        {
            SetClickThrough(false);
            ImGui::Begin("Lucid Aimbot | getlucid.vercel.app", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

            if (ImGui::BeginTabBar("MainTabs"))
            {
                if (ImGui::BeginTabItem("Home"))
                {
                    ImGui::BeginChild("Home", ImVec2(0, 134), true); // box;

                    ImGui::Text("Welcome %s", globals::PlrName);

                    ImGui::NewLine();
                    ImGui::Text("Place ID: %llu", globals::placeId);
                    ImGui::Text("Game ID: %llu", globals::GameId);
                    ImGui::Text("Job ID: %llu", globals::JobId);


                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Movement"))
                {
                    ImGui::BeginChild("MovementManip", ImVec2(0, 134), true); // box

                    ImGui::Text("Movement Manipulation");

                    ImGui::Checkbox("##walk_toggle", &gui::Walkspeed);
                    ImGui::SameLine();
                    ImGui::BeginDisabled(!gui::Walkspeed);
                    ImGui::SliderFloat("Walkspeed", &gui::WalkspeedValue, 1.0f, 400.0f, "%.0f");
                    ImGui::EndDisabled();

                    ImGui::Checkbox("##jump_toggle", &gui::JumpPower);
                    ImGui::SameLine();
                    ImGui::BeginDisabled(!gui::JumpPower);
                    ImGui::SliderFloat("Jump Power", &gui::JumpPowerValue, 1.0f, 500.0f, "%.0f");
                    ImGui::EndDisabled();

                    ImGui::Checkbox("##gravity_toggle", &gui::Gravity);
                    ImGui::SameLine();
                    ImGui::BeginDisabled(!gui::Gravity);
                    ImGui::SliderFloat("Gravity", &gui::GravityValue, 1.0f, 400.0f, "%.0f");
                    ImGui::EndDisabled();

                    ImGui::EndChild();

                    ImGui::Spacing();

                    ImGui::BeginChild("AdditionalMove", ImVec2(0, 100), true);
                    ImGui::Text("Additional Movement");


                    if (gui::Expirimental)
                        ImGui::Checkbox("Enable Noclip", &gui::noclip);
                    ImGui::BeginDisabled();
                    ImGui::Checkbox("##Fly_toggle", &gui::Fly);
                    ImGui::SameLine();
                    ImGui::BeginDisabled(!gui::Fly);
                    ImGui::SliderFloat("Fly", &gui::FlyValue, 1.0f, 100.0f, "%.1f");
                    ImGui::EndDisabled();

					ImGui::EndDisabled();
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Aimbot"))
                {
                    ImGui::BeginChild("FOV Crcl", ImVec2(0, 170), true);

                    ImGui::Text("FOV Circle");
                    ImGui::Checkbox("Show FOV", &aimbot::RenderFOV);

                    const char* colorModes[] = { "Custom Color", "Rainbow" };
                    ImGui::Combo("FOV Color type", &FOVcolorMode, colorModes, IM_ARRAYSIZE(colorModes));
                    if (FOVcolorMode == 0)
                    {
                        ImGui::ColorEdit4("FOV Color", (float*)&FOVColor, ImGuiColorEditFlags_NoAlpha);
                    }

                    ImGui::SliderFloat("FOV Size", &aimbot::FOVSize, 10.0f, 180.0f, "%.0f");
                    ImGui::EndChild();

                    static int selectedPart = 0;
                    static const char* hitParts[] = { "Head", "HumanoidRootPart" };

                    ImGui::BeginChild("AimbotMain", ImVec2(363, 200), true);
                    ImGui::Text("Mouse Aimbot");

                    static const char* LockBinds[] = { "Mouse Side Button Down", "Q", "E", "Z", "X", "Right Mouse Button" };
                    // Checkbox to enable/disable aimbot
                    ImGui::Checkbox("Enable Mouse bot", &gui::Aimbot);
                    static int SelectedBindInt = 0;
					ImGui::Text("Lock Keybind");
                    ImGui::Combo("##Lock Keybind", &SelectedBindInt, LockBinds, IM_ARRAYSIZE(LockBinds));

                    aimbot::LockKey = LockBinds[SelectedBindInt];

                    // Combo box for selecting target part (with unique label)
                    ImGui::Text("Hit Part");
                    ImGui::Combo("##HitPartCombo", &selectedPart, hitParts, IM_ARRAYSIZE(hitParts));

                    aimbot::targetPartName = hitParts[selectedPart];

                    ImGui::Text("Aimbot Smoothness");
                    ImGui::SliderFloat("##AimbotSmoothness", &aimbot::smoothness, 0.0f, 10.0f, "%.1f");

                    ImGui::EndChild();

					ImGui::SameLine();

					ImGui::BeginChild("Aimbot Checks", ImVec2(155, 200), true);

                    ImGui::Text("Checks");
                    ImGui::Checkbox("Team Check", &aimbot::TeamCheck);
                    ImGui::Checkbox("Dead Check", &aimbot::DeadCheck);

				    ImGui::EndChild();

                    ImGui::Spacing();

                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("ESP"))
                {
                    ImGui::BeginChild("ESPMain", ImVec2(259, 190), true);
                    ImGui::Text("ESP Features");
                    static bool healthBar = true;

                    ImGui::Text("Box ESP");
                    ImGui::Checkbox("Box ESP", &gui::ESP);
                    

                    ImGui::Text("Other");
                    ImGui::Checkbox("Tracers", &gui::tracers);
                    ImGui::Checkbox("Name Tag", &gui::NameEsp);
                    if (gui::Expirimental)
                        ImGui::Checkbox("Skeleton ESP", &gui::SkeletonESP);

                    ImGui::EndChild();

                    ImGui::SameLine();

                    ImGui::BeginChild("ESPOther", ImVec2(259, 190), true);
                    ImGui::Text("Checks");
                    ImGui::Checkbox("Enemy ESP", &gui::EnemyESP);
                    ImGui::Checkbox("Team ESP", &gui::TeamESP);
					ImGui::Text("--------------------------------");
                    ImGui::Checkbox("Health Check", &gui::HealthCheck);
                    ImGui::Checkbox("Within Screen", &gui::ScreenBounds);


                    ImGui::EndChild();  


                    ImGui::Spacing();

                    ImGui::BeginChild("ESPColor", ImVec2(0, 150), true);

                    ImGui::Text("Options");

                    const char* colorModes[] = { "Custom Color", "Rainbow" };
                    ImGui::Combo("Color type", &colorMode, colorModes, IM_ARRAYSIZE(colorModes));

                    if (colorMode == 0)
                    {
                        ImGui::ColorEdit4("ESP Color", (float*)&espColor, ImGuiColorEditFlags_NoAlpha);
                    }

                    ImGui::SliderFloat("Thickness", &gui::Thickness, 0.1f, 10.0f, "%.0f");

                    ImGui::EndChild();


                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Explorer"))
                {
                    ListExplorer(globals::datamodel);
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("Settings"))
                {
                    ImGui::BeginChild("SettingsGroup", ImVec2(0, 167), true);

                    static bool startMinimized = false;


                    ImGui::Text("Settings");
                    ImGui::Checkbox("Show Watermark", &showWatermark);
                    ImGui::Checkbox("Show Console", &gui::ShowConsole);
					ImGui::Checkbox("Expirimental Features", &gui::Expirimental);

                    static bool lastShowConsole = gui::ShowConsole;

                    if (gui::ShowConsole != lastShowConsole)
                    {
                        HWND consoleWindow = GetConsoleWindow();
                        if (consoleWindow != nullptr)
                        {
                            ShowWindow(consoleWindow, gui::ShowConsole ? SW_SHOW : SW_HIDE);
                        }
                        lastShowConsole = gui::ShowConsole;
                    }

                    ImGui::Spacing();
                    if (ImGui::Button("Unload UI"))
                    {
                        if (globals::hProc != 0)
                            CloseHandle(globals::hProc);
                        done = true;
                    }

                    ImGui::SameLine();
                    ImGui::EndChild();

                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            ImGui::End();
        }
        else
            SetClickThrough(true);

        if (showWatermark)
        {
            ImGui::SetNextWindowBgAlpha(0.4f);

            ImVec2 display_size = ImGui::GetIO().DisplaySize;

            ImVec2 window_size = ImVec2(320, 35);

            float padding = 15.0f;
            ImGui::SetNextWindowPos(ImVec2(padding, display_size.y - window_size.y - padding), ImGuiCond_Always);

            const char* watermarkName = "##Watermark";

            ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);

            ImGui::Begin(watermarkName, nullptr,
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_AlwaysAutoResize |
                ImGuiWindowFlags_NoNav |
                ImGuiWindowFlags_NoDecoration);

            const char* text = "Lucid | Status: Undetected | FPS: %.0f";
            char buf[256];
            snprintf(buf, sizeof(buf), text, ImGui::GetIO().Framerate);

            // Calculate text size
            ImVec2 text_size = ImGui::CalcTextSize(buf);
            float window_width = ImGui::GetWindowWidth();

            // Set cursor position to center text horizontally
            ImGui::SetCursorPosX((window_width - text_size.x) * 0.5f);

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));
            ImGui::TextUnformatted(buf);
            ImGui::PopStyleColor();

            ImGui::End();
        }

        MakeAimbot();
        DrawESP();
        ImGui::EndFrame();

        
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        // Present
        HRESULT hr = g_pSwapChain->Present(0, 0);   // Present without vsync

        //HRESULT hr = g_pSwapChain->Present(0, 0); // Present without vsync
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(imgui::hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);

    return 0;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
