
#ifndef MYDIRECT_SOURCE_RENDERER_RENDERER_H
#define MYDIRECT_SOURCE_RENDERER_RENDERER_H


#include "../Common/Singleton.h"
#include "../Common/d3dUtil.h"

class Renderer : public Singleton<Renderer>
{
public:
    Renderer();
    ~Renderer();

    bool Init();
    void Draw();

private:
    void CreateCommandObjects();
    void CreateSwapChain();

    void LogAdapters();
    void LogAdapterOutputs(IDXGIAdapter* in_adapter);
    void LogOutputDisplayModes(IDXGIOutput* in_output, DXGI_FORMAT in_format);

    bool      m_4x_msaa_state = false;    // 4X MSAA enabled

    UINT      m_4x_msaa_quality = 0;      // quality level of 4X MSAA

    Microsoft::WRL::ComPtr<IDXGIFactory4> m_dxgi_factory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swap_chain;
    Microsoft::WRL::ComPtr<ID3D12Device> m_d3d_device;

    Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
    UINT64 m_current_fence = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_direct_cmd_list_alloc;

    static const int SwapChainBufferCount = 2;

    UINT m_rtv_descriptor_size = 0;
    UINT m_dsv_descriptor_size = 0;
    UINT m_cbv_srv_uav_descriptor_size = 0;

    DXGI_FORMAT m_back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;

    int m_client_width = 800;
    int m_client_height = 600;
};



#endif  // MYDIRECT_SOURCE_RENDERER_RENDERER_H

