#include "panel.hpp"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

static constexpr std::uintptr_t GTA_DEVICE_PTR = 0xC97C28; // IDirect3DDevice9**
static constexpr std::uintptr_t GTA_HWND_PTR   = 0xC97C1C; // HWND*

IDirect3DDevice9* panel::getDevice() const noexcept {
    const auto ppDevice = reinterpret_cast<IDirect3DDevice9**>(GTA_DEVICE_PTR);
    return (ppDevice ? *ppDevice : nullptr);
}

HWND panel::getHwnd() const noexcept {
    return *reinterpret_cast<HWND*>(GTA_HWND_PTR);
}

panel::panel() {
    IDirect3DDevice9* pDevice = getDevice();
    if (!pDevice) return;

    // The IDirect3DDevice9 vtable starts at *pDevice
    // Reset   = slot 16
    // Present = slot 17
    const auto vt = *reinterpret_cast<std::uintptr_t**>(pDevice);

    hookReset_   = std::make_unique<lemon::detour<oReset>>  (vt[16]);
    hookPresent_ = std::make_unique<lemon::detour<oPresent>>(vt[17]);

    hookReset_  ->install(std::make_tuple(this, &panel::hookedReset));
    hookPresent_->install(std::make_tuple(this, &panel::hookedPresent));
}

panel::~panel() {
    imguiDestroy();
    if (hookPresent_) hookPresent_->remove();
    if (hookReset_)   hookReset_->remove();
}

HRESULT panel::hookedReset(oReset orig,
                            IDirect3DDevice9* pDevice,
                            D3DPRESENT_PARAMETERS* pPP) {
    imguiDestroy();
    const HRESULT hr = orig(pDevice, pPP);
    if (SUCCEEDED(hr)) {
        imguiCreate(pDevice);
    }
    return hr;
}

HRESULT panel::hookedPresent(oPresent orig,
                              IDirect3DDevice9* pDevice,
                              const RECT* pSrc, const RECT* pDest,
                              HWND hDest, const RGNDATA* pDirty) {
    static bool ready = false;
    if (!ready) {
        imguiCreate(pDevice);
        ready = true;
    }

    if (menuOpen) {
        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        imguiRender();

        ImGui::EndFrame();
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
    }

    return orig(pDevice, pSrc, pDest, hDest, pDirty);
}

void panel::imguiCreate(IDirect3DDevice9* pDevice) const {
    if (ImGui::GetCurrentContext()) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_Init(getHwnd());
    ImGui_ImplDX9_Init(pDevice);
    imguiStyle();
}

void panel::imguiDestroy() const {
    if (!ImGui::GetCurrentContext()) return;
    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void panel::imguiStyle() const {
    ImGuiStyle& s = ImGui::GetStyle();
    ImVec4* c = s.Colors;

    s.WindowRounding    = 6.f;
    s.FrameRounding     = 4.f;
    s.ScrollbarRounding = 4.f;
    s.GrabRounding      = 4.f;
    s.ChildRounding     = 4.f;
    s.WindowBorderSize  = 1.f;
    s.FrameBorderSize   = 0.f;
    s.ItemSpacing       = { 8.f, 6.f };

    c[ImGuiCol_WindowBg]         = { 0.10f, 0.10f, 0.13f, 0.95f };
    c[ImGuiCol_TitleBg]          = { 0.07f, 0.07f, 0.09f, 1.00f };
    c[ImGuiCol_TitleBgActive]    = { 0.07f, 0.07f, 0.09f, 1.00f };
    c[ImGuiCol_FrameBg]          = { 0.16f, 0.16f, 0.20f, 1.00f };
    c[ImGuiCol_FrameBgHovered]   = { 0.22f, 0.22f, 0.28f, 1.00f };
    c[ImGuiCol_FrameBgActive]    = { 0.18f, 0.18f, 0.24f, 1.00f };
    c[ImGuiCol_Header]           = { 0.18f, 0.18f, 0.24f, 1.00f };
    c[ImGuiCol_HeaderHovered]    = { 0.00f, 0.70f, 0.80f, 0.30f };
    c[ImGuiCol_HeaderActive]     = { 0.00f, 0.70f, 0.80f, 0.50f };
    c[ImGuiCol_Button]           = { 0.16f, 0.16f, 0.20f, 1.00f };
    c[ImGuiCol_ButtonHovered]    = { 0.00f, 0.70f, 0.80f, 0.60f };
    c[ImGuiCol_ButtonActive]     = { 0.00f, 0.55f, 0.65f, 1.00f };
    c[ImGuiCol_Tab]              = { 0.13f, 0.13f, 0.17f, 1.00f };
    c[ImGuiCol_TabHovered]       = { 0.00f, 0.70f, 0.80f, 0.70f };
    c[ImGuiCol_TabActive]        = { 0.00f, 0.60f, 0.70f, 1.00f };
    c[ImGuiCol_SliderGrab]       = { 0.00f, 0.70f, 0.80f, 0.90f };
    c[ImGuiCol_SliderGrabActive] = { 0.00f, 0.85f, 0.95f, 1.00f };
    c[ImGuiCol_CheckMark]        = { 0.00f, 0.85f, 0.95f, 1.00f };
    c[ImGuiCol_Separator]        = { 0.25f, 0.25f, 0.30f, 1.00f };
    c[ImGuiCol_Border]           = { 0.25f, 0.25f, 0.32f, 0.80f };
    c[ImGuiCol_Text]             = { 0.90f, 0.90f, 0.95f, 1.00f };
    c[ImGuiCol_TextDisabled]     = { 0.50f, 0.50f, 0.55f, 1.00f };
    c[ImGuiCol_ScrollbarBg]      = { 0.10f, 0.10f, 0.13f, 1.00f };
    c[ImGuiCol_ScrollbarGrab]    = { 0.25f, 0.25f, 0.30f, 1.00f };
    c[ImGuiCol_PopupBg]          = { 0.10f, 0.10f, 0.13f, 0.98f };
}

void panel::imguiRender() const {
    ImGui::SetNextWindowSize({ 420.f, 280.f }, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos ({ 100.f, 100.f }, ImGuiCond_FirstUseEver);

    ImGui::Begin("## Panel", nullptr,
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

    ImGui::TextColored({ 0.0f, 0.85f, 0.95f, 1.0f }, "  SAMP Panel");
    ImGui::SameLine(ImGui::GetWindowWidth() - 70.f);
    ImGui::TextDisabled("v1.0");
    ImGui::Separator();
    ImGui::Spacing();

    if (ImGui::BeginTabBar("##tabs")) {
        if (ImGui::BeginTabItem("Main"))   { tabMain();  ImGui::EndTabItem(); }
        if (ImGui::BeginTabItem("About")) { tabAbout(); ImGui::EndTabItem(); }
        ImGui::EndTabBar();
    }

    ImGui::End();
}

void panel::tabMain() const {
    static bool  featureA  = false;
    static bool  featureB  = false;
    static int   sliderVal = 50;
    static char  inputBuf[64]{};

    ImGui::Spacing();
    ImGui::TextDisabled("Test Options");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("Feature A (example)", &featureA);
    ImGui::Checkbox("Feature B (example)", &featureB);

    ImGui::Spacing();
    ImGui::SliderInt("Value", &sliderVal, 0, 100);

    ImGui::Spacing();
    ImGui::SetNextItemWidth(200.f);
    ImGui::InputText("Input", inputBuf, sizeof(inputBuf));

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    if (featureA || featureB)
        ImGui::TextColored({ 0.0f, 0.95f, 0.4f, 1.0f }, "[*] Some active feature");
    else
        ImGui::TextDisabled("[*] No active features");

    ImGui::Spacing();
    if (ImGui::Button("Test Button", { 160.f, 28.f })) {
    }
}

void panel::tabAbout() const {
    ImGui::Spacing();
    ImGui::TextColored({ 0.0f, 0.85f, 0.95f, 1.0f }, "SAMP Panel - Test Panel");
    ImGui::Spacing();
    ImGui::TextWrapped(
        "Basic panel for SAMP built with ImGui + DirectX 9.\n"
        "Open/close with the INSERT key.");
    ImGui::Spacing();
    ImGui::Separator();
}
