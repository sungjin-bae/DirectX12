
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
    void LogAdapterOutputs(IDXGIAdapter* adapter);
    void LogOutputDisplayModes(IDXGIOutput* output, DXGI_FORMAT format);

    bool      m4xMsaaState = false;    // 4X MSAA enabled

    UINT      m4xMsaaQuality = 0;      // quality level of 4X MSAA

    Microsoft::WRL::ComPtr<IDXGIFactory4> mdxgiFactory;
    Microsoft::WRL::ComPtr<IDXGISwapChain> mSwapChain;
    Microsoft::WRL::ComPtr<ID3D12Device> md3dDevice;

    Microsoft::WRL::ComPtr<ID3D12Fence> mFence;
    UINT64 mCurrentFence = 0;

    Microsoft::WRL::ComPtr<ID3D12CommandAllocator> mDirectCmdListAlloc;

    static const int SwapChainBufferCount = 2;

    UINT mRtvDescriptorSize = 0;
    UINT mDsvDescriptorSize = 0;
    UINT mCbvSrvUavDescriptorSize = 0;

    DXGI_FORMAT mBackBufferFormat = DXGI_FORMAT_R8G8B8A8_UNORM;

    int mClientWidth = 800;
    int mClientHeight = 600;
};



#endif  // MYDIRECT_SOURCE_RENDERER_RENDERER_H

