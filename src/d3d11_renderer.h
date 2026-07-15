#pragma once

#include <windows.h>
#include <d3d11.h>
#include <wrl/client.h>

// Owns a standalone D3D11 device and an offscreen render target that is
// submitted to SteamVR as the dashboard overlay texture. This device does
// not need to match the HMD's graphics adapter; overlay textures are
// composited by SteamVR independently of the eye render path.
class D3D11Renderer {
public:
    bool Init(int width, int height);
    void Shutdown();

    // Binds the offscreen texture as the current render target and clears it.
    void BeginFrame(float r, float g, float b, float a);

    bool Resize(int width, int height);

    ID3D11Device* Device() const { return m_device.Get(); }
    ID3D11DeviceContext* Context() const { return m_context.Get(); }
    ID3D11Texture2D* OverlayTexture() const { return m_texture.Get(); }

    int Width() const { return m_width; }
    int Height() const { return m_height; }

private:
    bool CreateTargets(int width, int height);
    void ReleaseTargets();

    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;

    int m_width = 0;
    int m_height = 0;
};
