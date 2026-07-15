#include "d3d11_renderer.h"

bool D3D11Renderer::Init(int width, int height) {
    UINT createFlags = 0;
#if defined(_DEBUG)
    createFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL requestedLevels[] = { D3D_FEATURE_LEVEL_11_0 };
    D3D_FEATURE_LEVEL obtainedLevel{};

    HRESULT hr = D3D11CreateDevice(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createFlags,
        requestedLevels,
        _countof(requestedLevels),
        D3D11_SDK_VERSION,
        m_device.GetAddressOf(),
        &obtainedLevel,
        m_context.GetAddressOf());

    if (FAILED(hr)) {
        return false;
    }

    return CreateTargets(width, height);
}

bool D3D11Renderer::CreateTargets(int width, int height) {
    ReleaseTargets();

    D3D11_TEXTURE2D_DESC desc{};
    desc.Width = static_cast<UINT>(width);
    desc.Height = static_cast<UINT>(height);
    desc.MipLevels = 1;
    desc.ArraySize = 1;
    desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage = D3D11_USAGE_DEFAULT;
    desc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;

    HRESULT hr = m_device->CreateTexture2D(&desc, nullptr, m_texture.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device->CreateRenderTargetView(m_texture.Get(), nullptr, m_rtv.GetAddressOf());
    if (FAILED(hr)) {
        return false;
    }

    m_width = width;
    m_height = height;
    return true;
}

void D3D11Renderer::ReleaseTargets() {
    m_rtv.Reset();
    m_texture.Reset();
}

bool D3D11Renderer::Resize(int width, int height) {
    if (width == m_width && height == m_height) {
        return true;
    }
    return CreateTargets(width, height);
}

void D3D11Renderer::BeginFrame(float r, float g, float b, float a) {
    ID3D11RenderTargetView* rtv = m_rtv.Get();
    m_context->OMSetRenderTargets(1, &rtv, nullptr);

    D3D11_VIEWPORT vp{};
    vp.Width = static_cast<float>(m_width);
    vp.Height = static_cast<float>(m_height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);

    const float clearColor[4] = { r, g, b, a };
    m_context->ClearRenderTargetView(m_rtv.Get(), clearColor);
}

void D3D11Renderer::Shutdown() {
    ReleaseTargets();
    m_context.Reset();
    m_device.Reset();
}
