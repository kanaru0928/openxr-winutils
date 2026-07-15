#include "gui.h"

#include "restart_advsettings.h"

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>

#include <mutex>
#include <thread>

namespace gui {

namespace {
std::mutex g_statusMutex;
}

void Init(ID3D11Device* device, ID3D11DeviceContext* context) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;

    ImGui::StyleColorsDark();

    ImGui_ImplDX11_Init(device, context);
}

void Shutdown() {
    ImGui_ImplDX11_Shutdown();
    ImGui::DestroyContext();
}

void NewFrame(int width, int height, float deltaTime) {
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
    io.DeltaTime = deltaTime > 0.0f ? deltaTime : (1.0f / 90.0f);

    ImGui_ImplDX11_NewFrame();
    ImGui::NewFrame();
}

void Render(UiState& state) {
    const ImGuiIO& io = ImGui::GetIO();

    ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::SetNextWindowSize(io.DisplaySize);

    const ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin("OpenXR WinUtils", nullptr, flags);

    ImGui::TextUnformatted("OpenXR WinUtils");
    ImGui::Separator();
    ImGui::Spacing();

    bool restarting = state.restartInProgress.load();
    ImGui::BeginDisabled(restarting);
    if (ImGui::Button(restarting ? "Restarting..." : "Restart OVR Advanced Settings",
        ImVec2(340.0f, 48.0f))) {
        state.restartInProgress.store(true);
        {
            std::lock_guard<std::mutex> lock(g_statusMutex);
            state.statusText = "Terminating AdvancedSettings.exe and relaunching...";
        }

        UiState* statePtr = &state;
        std::thread([statePtr]() {
            restart_advsettings::Result result = restart_advsettings::Restart();
            {
                std::lock_guard<std::mutex> lock(g_statusMutex);
                statePtr->statusText = result.message;
            }
            statePtr->restartInProgress.store(false);
        }).detach();
    }
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Status:");
    {
        std::lock_guard<std::mutex> lock(g_statusMutex);
        ImGui::TextWrapped("%s", state.statusText.c_str());
    }

    ImGui::End();
}

} // namespace gui
