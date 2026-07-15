#pragma once

#include <atomic>
#include <string>

struct ID3D11Device;
struct ID3D11DeviceContext;

namespace gui {

struct UiState {
    std::atomic<bool> restartInProgress{ false };
    std::string statusText = "Ready.";
};

void Init(ID3D11Device* device, ID3D11DeviceContext* context);
void Shutdown();

// Must be called once per frame before Render(), with the current overlay
// texture size and elapsed time since the previous frame.
void NewFrame(int width, int height, float deltaTime);

// Builds and submits the ImGui draw data for the current frame. Does not
// call ImGui_ImplDX11_RenderDrawData(); the caller does that after binding
// the offscreen render target.
void Render(UiState& state);

} // namespace gui
