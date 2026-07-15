#include "overlay_app.h"

#include <windows.h>

#include <filesystem>
#include <string>
#include <thread>

#include <imgui.h>
#include <backends/imgui_impl_dx11.h>

namespace {

std::string WideToUtf8(const std::wstring& wide) {
    if (wide.empty()) {
        return {};
    }
    int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) {
        return {};
    }
    std::string result(static_cast<size_t>(size), '\0');
    WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), -1, result.data(), size, nullptr, nullptr);
    if (!result.empty() && result.back() == '\0') {
        result.pop_back();
    }
    return result;
}

std::filesystem::path ExecutableDirectory() {
    wchar_t buffer[MAX_PATH] = {};
    GetModuleFileNameW(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(buffer).parent_path();
}

} // namespace

bool OverlayApp::Init() {
    vr::EVRInitError initError = vr::VRInitError_None;
    vr::VR_Init(&initError, vr::VRApplication_Overlay);
    if (initError != vr::VRInitError_None) {
        return false;
    }
    m_vrInitialized = true;

    RegisterManifestIfNeeded();

    vr::EVROverlayError overlayError = vr::VROverlay()->CreateDashboardOverlay(
        kAppKey, kAppName, &m_overlayHandle, &m_thumbnailHandle);
    if (overlayError != vr::VROverlayError_None) {
        return false;
    }

    vr::VROverlay()->SetOverlayWidthInMeters(m_overlayHandle, 2.0f);
    vr::VROverlay()->SetOverlayInputMethod(m_overlayHandle, vr::VROverlayInputMethod_Mouse);

    vr::HmdVector2_t mouseScale;
    mouseScale.v[0] = static_cast<float>(kOverlayWidth);
    mouseScale.v[1] = static_cast<float>(kOverlayHeight);
    vr::VROverlay()->SetOverlayMouseScale(m_overlayHandle, &mouseScale);

    std::filesystem::path iconPath = ExecutableDirectory() / L"icon.png";
    std::string iconPathUtf8 = WideToUtf8(iconPath.wstring());
    vr::VROverlay()->SetOverlayFromFile(m_thumbnailHandle, iconPathUtf8.c_str());

    if (!m_renderer.Init(kOverlayWidth, kOverlayHeight)) {
        return false;
    }

    gui::Init(m_renderer.Device(), m_renderer.Context());
    m_imguiInitialized = true;

    m_lastFrameTime = std::chrono::steady_clock::now();
    return true;
}

void OverlayApp::RegisterManifestIfNeeded() {
    vr::IVRApplications* apps = vr::VRApplications();
    if (apps == nullptr) {
        return;
    }

    if (apps->IsApplicationInstalled(kAppKey)) {
        return;
    }

    std::filesystem::path manifestPath = ExecutableDirectory() / L"manifest.vrmanifest";
    std::string manifestPathUtf8 = WideToUtf8(manifestPath.wstring());
    apps->AddApplicationManifest(manifestPathUtf8.c_str(), false);
}

void OverlayApp::ProcessSystemEvents() {
    vr::IVRSystem* system = vr::VRSystem();
    if (system == nullptr) {
        return;
    }

    vr::VREvent_t event;
    while (system->PollNextEvent(&event, sizeof(event))) {
        if (event.eventType == vr::VREvent_Quit) {
            system->AcknowledgeQuit_Exiting();
            m_running = false;
        }
    }
}

void OverlayApp::ProcessOverlayEvents() {
    vr::IVROverlay* overlay = vr::VROverlay();
    if (overlay == nullptr) {
        return;
    }

    ImGuiIO& io = ImGui::GetIO();
    vr::VREvent_t event;

    while (overlay->PollNextOverlayEvent(m_overlayHandle, &event, sizeof(event))) {
        switch (event.eventType) {
            case vr::VREvent_MouseMove: {
                float x = event.data.mouse.x;
                float y = static_cast<float>(kOverlayHeight) - event.data.mouse.y;
                io.AddMousePosEvent(x, y);
                break;
            }
            case vr::VREvent_MouseButtonDown:
            case vr::VREvent_MouseButtonUp: {
                const bool down = (event.eventType == vr::VREvent_MouseButtonDown);
                const uint32_t button = event.data.mouse.button;
                if (button & vr::VRMouseButton_Left) {
                    io.AddMouseButtonEvent(ImGuiMouseButton_Left, down);
                }
                if (button & vr::VRMouseButton_Right) {
                    io.AddMouseButtonEvent(ImGuiMouseButton_Right, down);
                }
                if (button & vr::VRMouseButton_Middle) {
                    io.AddMouseButtonEvent(ImGuiMouseButton_Middle, down);
                }
                break;
            }
            case vr::VREvent_ScrollDiscrete: {
                io.AddMouseWheelEvent(event.data.scroll.xdelta, event.data.scroll.ydelta);
                break;
            }
            case vr::VREvent_Quit: {
                vr::VRSystem()->AcknowledgeQuit_Exiting();
                m_running = false;
                break;
            }
            default:
                break;
        }
    }
}

void OverlayApp::SubmitTexture() {
    vr::Texture_t texture{};
    texture.handle = static_cast<void*>(m_renderer.OverlayTexture());
    texture.eType = vr::TextureType_DirectX;
    texture.eColorSpace = vr::ColorSpace_Auto;
    vr::VROverlay()->SetOverlayTexture(m_overlayHandle, &texture);
}

bool OverlayApp::Tick() {
    ProcessSystemEvents();
    if (!m_running) {
        return false;
    }

    ProcessOverlayEvents();
    if (!m_running) {
        return false;
    }

    const bool dashboardVisible = vr::VROverlay()->IsDashboardVisible();

    if (dashboardVisible) {
        auto now = std::chrono::steady_clock::now();
        float deltaTime = std::chrono::duration<float>(now - m_lastFrameTime).count();
        m_lastFrameTime = now;

        gui::NewFrame(m_renderer.Width(), m_renderer.Height(), deltaTime);
        gui::Render(m_uiState);
        ImGui::Render();

        m_renderer.BeginFrame(0.0f, 0.0f, 0.0f, 0.0f);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        SubmitTexture();
    } else {
        m_lastFrameTime = std::chrono::steady_clock::now();
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(dashboardVisible ? 11 : 150));
    return m_running;
}

void OverlayApp::Shutdown() {
    if (m_imguiInitialized) {
        gui::Shutdown();
        m_imguiInitialized = false;
    }

    m_renderer.Shutdown();

    if (m_vrInitialized) {
        vr::VR_Shutdown();
        m_vrInitialized = false;
    }
}
