#pragma once

#include <chrono>

#include <openvr.h>

#include "d3d11_renderer.h"
#include "gui.h"

// Owns the OpenVR session, the dashboard overlay, and the offscreen D3D11
// renderer. Drives the ImGui frame and feeds overlay input events into it.
class OverlayApp {
public:
    bool Init();
    void Shutdown();

    // Pumps OpenVR events, renders a frame when appropriate, and submits the
    // result as the overlay texture. Returns false once SteamVR requests
    // that the application quit.
    bool Tick();

private:
    void RegisterManifestIfNeeded();
    void ProcessSystemEvents();
    void ProcessOverlayEvents();
    void SubmitTexture();

    static constexpr int kOverlayWidth = 800;
    static constexpr int kOverlayHeight = 600;
    static constexpr const char* kAppKey = "kanaru.openxr-winutils";
    static constexpr const char* kAppName = "OpenXR WinUtils";

    D3D11Renderer m_renderer;
    gui::UiState m_uiState;

    vr::VROverlayHandle_t m_overlayHandle = vr::k_ulOverlayHandleInvalid;
    vr::VROverlayHandle_t m_thumbnailHandle = vr::k_ulOverlayHandleInvalid;

    bool m_running = true;
    bool m_vrInitialized = false;
    bool m_imguiInitialized = false;

    std::chrono::steady_clock::time_point m_lastFrameTime;
};
