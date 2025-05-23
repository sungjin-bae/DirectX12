#pragma once

#include "../Common/Singleton.h"
#include "../Common/d3dUtil.h"
#include "../Common/UploadBuffer.h"
#include "../RenderObject/RenderObject.h"

// 셰이더에서 사용할 상수 버퍼 구조체
struct ObjectConstants
{
    DirectX::XMFLOAT4X4 m_world = MathHelper::Identity4x4();
};

class Renderer : public Singleton<Renderer>
{
public:
    Renderer();
    virtual ~Renderer();

    bool Init(HWND in_h_window);
    void Draw();

    void BuildPSO(const RenderObjSharedPtr& in_render_object);

    // 렌더링 헬퍼 함수들
    ID3D12Resource* CurrentBackBuffer() const;
    D3D12_CPU_DESCRIPTOR_HANDLE CurrentBackBufferView() const;
    D3D12_CPU_DESCRIPTOR_HANDLE DepthStencilView() const;
    void FlushCommandQueue();

private:
    void CreateCommandObjects();
    void CreateSwapChain();
    void CreateRtvAndDsvDescriptorHeaps();
    void BuildDescriptorHeaps();
    void BuildConstantBuffers();
    void BuildRootSignature();

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

    // CommandList를 제출해서 GPU가 실행
    Microsoft::WRL::ComPtr<ID3D12CommandQueue> m_command_queue;
    // 명령어를 저장할 메모리 제공
    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_direct_cmd_list_alloc;
    // 명령어를 저장하고 녹화(Record)
    Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_command_list;

    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtv_heap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_dsv_heap;
    Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_cbv_heap;

    Microsoft::WRL::ComPtr<ID3D12PipelineState> m_pso;
    Microsoft::WRL::ComPtr<ID3D12RootSignature> m_root_signature;

    std::vector<D3D12_INPUT_ELEMENT_DESC> m_input_layout;
    Microsoft::WRL::ComPtr<ID3DBlob> m_vs_byte_code;
    Microsoft::WRL::ComPtr<ID3DBlob> m_ps_byte_code;

    // 상수 버퍼
    std::unique_ptr<UploadBuffer<ObjectConstants>> m_object_constants_buffer;

    static const int swap_chain_buffer_count = 2;
    int m_curr_back_buffer = 0;

    // 스왑 체인 버퍼
    Microsoft::WRL::ComPtr<ID3D12Resource> m_swap_chain_buffer[swap_chain_buffer_count];

    D3D12_VIEWPORT m_screen_viewport;
    D3D12_RECT m_scissor_rect;

    UINT m_rtv_descriptor_size = 0;
    UINT m_dsv_descriptor_size = 0;
    UINT m_cbv_srv_uav_descriptor_size = 0;

    DXGI_FORMAT m_back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;

    int m_client_width = 800;
    int m_client_height = 600;

    HWND m_h_window;
};

