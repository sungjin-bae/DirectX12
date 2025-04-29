#include "../Common/d3dApp.h"

#include "Renderer.h"


Renderer::Renderer() {}
Renderer::~Renderer() {}

bool Renderer::Init(HWND in_h_window)
{
	AssertHRESULT(in_h_window);

	m_h_window = in_h_window;

#if defined(DEBUG) || defined(_DEBUG) 
	// Enable the D3D12 debug layer.
	{
		Microsoft::WRL::ComPtr<ID3D12Debug> debug_controller;
		AssertHRESULT(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
		debug_controller->EnableDebugLayer();
	}
#endif

	AssertHRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&m_dxgi_factory)));

	// Try to create hardware device.
	HRESULT hardwareResult = D3D12CreateDevice(
		nullptr,             // default adapter
		D3D_FEATURE_LEVEL_11_0,
		IID_PPV_ARGS(&m_d3d_device));

	// Fallback to WARP device.
	if(FAILED(hardwareResult))
	{
		Microsoft::WRL::ComPtr<IDXGIAdapter> pWarpAdapter;
		AssertHRESULT(m_dxgi_factory->EnumWarpAdapter(IID_PPV_ARGS(&pWarpAdapter)));

		AssertHRESULT(D3D12CreateDevice(
			pWarpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_d3d_device)));
	}

	AssertHRESULT(m_d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE,
		IID_PPV_ARGS(&m_fence)));

	m_rtv_descriptor_size = m_d3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	m_dsv_descriptor_size = m_d3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	m_cbv_srv_uav_descriptor_size = m_d3d_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

    // Check 4X MSAA quality support for our back buffer format.
    // All Direct3D 11 capable devices support 4X MSAA for all render 
    // target formats, so we only need to check quality support.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = m_back_buffer_format;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	AssertHRESULT(m_d3d_device->CheckFeatureSupport(
		D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
		&msQualityLevels,
		sizeof(msQualityLevels)));

    m_4x_msaa_quality = msQualityLevels.NumQualityLevels;
	assert(m_4x_msaa_quality > 0 && "Unexpected MSAA quality level.");
	
#ifdef _DEBUG
    LogAdapters();
#endif

	CreateCommandObjects();
    CreateSwapChain();
    CreateRtvAndDsvDescriptorHeaps();

	// [함수명]                   [설명]                                          [쉐이더 변경 영향 여부]
	// BuildDescriptorHeaps()     : CBV/SRV/UAV DescriptorHeap 생성                 - 거의 영향 없음 (※ 리소스 종류 추가 시 영향)
	// BuildConstantBuffers()     : ConstantBuffer 리소스 생성                     - 거의 영향 없음 (※ CB 구조 변경 시 영향)
	// BuildRootSignature()       : RootSignature 설정 (리소스 바인딩 규칙 정의)     - 영향 있음
	// BuildShadersAndInputLayout(): Shader 컴파일 + InputLayout(Vertex 포맷) 정의   - 영향 있음
	// BuildPSO()                 : PipelineStateObject 생성 (Shader, Rasterizer 등) - 영향 있음

	// BuildConstantBuffers();
    // BuildRootSignature();
    // BuildShadersAndInputLayout();
    // BuildBoxGeometry();
    // BuildPSO();

	return true;
}

void Renderer::Draw()
{
	// 명령어 기록과 관련된 메모리를 재사용합니다.
    // 명령어 리스트가 GPU에서 실행이 완료된 후에만 리셋할 수 있습니다.
	AssertHRESULT(m_direct_cmd_list_alloc->Reset());

	// 명령어 리스트는 ExecuteCommandList를 통해 명령어 큐에 추가된 후에 리셋할 수 있습니다.
    // 명령어 리스트를 재사용하면 메모리를 재사용할 수 있습니다.
    AssertHRESULT(m_command_list->Reset(m_direct_cmd_list_alloc.Get(), m_pso.Get()));
}

void Renderer::CreateCommandObjects()
{
	D3D12_COMMAND_QUEUE_DESC queue_desc = {};
	queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	AssertHRESULT(m_d3d_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_command_queue)));

	AssertHRESULT(m_d3d_device->CreateCommandAllocator(
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(m_direct_cmd_list_alloc.GetAddressOf())));

	AssertHRESULT(m_d3d_device->CreateCommandList(
		0,
		D3D12_COMMAND_LIST_TYPE_DIRECT,
		m_direct_cmd_list_alloc.Get(),
		nullptr,
		IID_PPV_ARGS(m_command_list.GetAddressOf())));

	// 명령어 리스트는 생성 시점에 열린 상태로 생성되므로 닫아줍니다.
	m_command_list->Close();
}

void Renderer::CreateSwapChain()
{
	// Release the previous swapchain we will be recreating.
	m_swap_chain.Reset();

	DXGI_SWAP_CHAIN_DESC swap_chain_desc;
	swap_chain_desc.BufferDesc.Width = m_client_width;
	swap_chain_desc.BufferDesc.Height = m_client_height;
	swap_chain_desc.BufferDesc.RefreshRate.Numerator = 60;
	swap_chain_desc.BufferDesc.RefreshRate.Denominator = 1;
	swap_chain_desc.BufferDesc.Format = m_back_buffer_format;
	swap_chain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	swap_chain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	swap_chain_desc.SampleDesc.Count = m_4x_msaa_state ? 4 : 1;
	swap_chain_desc.SampleDesc.Quality = m_4x_msaa_state ? (m_4x_msaa_quality - 1) : 0;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.BufferCount = swap_chain_buffer_count;
	swap_chain_desc.OutputWindow = m_h_window;
	swap_chain_desc.Windowed = true;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swap_chain_desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	// Note: Swap chain uses queue to perform flush.
	AssertHRESULT(m_dxgi_factory->CreateSwapChain(
		m_command_queue.Get(),
		&swap_chain_desc,
		m_swap_chain.GetAddressOf()));
}

void Renderer::CreateRtvAndDsvDescriptorHeaps()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc;
	rtv_heap_desc.NumDescriptors = swap_chain_buffer_count;
	rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtv_heap_desc.NodeMask = 0;
	AssertHRESULT(m_d3d_device->CreateDescriptorHeap(
		&rtv_heap_desc, IID_PPV_ARGS(m_rtv_heap.GetAddressOf())));


	D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_desc;
	dsv_heap_desc.NumDescriptors = 1;
	dsv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsv_heap_desc.NodeMask = 0;
	AssertHRESULT(m_d3d_device->CreateDescriptorHeap(
		&dsv_heap_desc, IID_PPV_ARGS(m_dsv_heap.GetAddressOf())));
}

void Renderer::LogAdapters()
{
	UINT i = 0;
	IDXGIAdapter* adapter = nullptr;
	std::vector<IDXGIAdapter*> adapter_list;
	while (m_dxgi_factory->EnumAdapters(i, &adapter) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_ADAPTER_DESC desc;
		adapter->GetDesc(&desc);

		std::wstring text = L"***Adapter: ";
		text += desc.Description;
		text += L"\n";

		OutputDebugString(text.c_str());

		adapter_list.push_back(adapter);

		++i;
	}

	for (size_t i = 0; i < adapter_list.size(); ++i)
	{
		LogAdapterOutputs(adapter_list[i]);
		ReleaseCom(adapter_list[i]);
	}
}

void Renderer::LogAdapterOutputs(IDXGIAdapter* in_adapter)
{
	UINT i = 0;
	IDXGIOutput* output = nullptr;
	while (in_adapter->EnumOutputs(i, &output) != DXGI_ERROR_NOT_FOUND)
	{
		DXGI_OUTPUT_DESC desc;
		output->GetDesc(&desc);

		std::wstring text = L"***Output: ";
		text += desc.DeviceName;
		text += L"\n";
		OutputDebugString(text.c_str());

		LogOutputDisplayModes(output, m_back_buffer_format);

		ReleaseCom(output);

		++i;
	}
}

void Renderer::LogOutputDisplayModes(IDXGIOutput* in_output, DXGI_FORMAT in_format)
{
	UINT count = 0;
	UINT flags = 0;

	// Call with nullptr to get list count.
	in_output->GetDisplayModeList(in_format, flags, &count, nullptr);

	std::vector<DXGI_MODE_DESC> mode_list(count);
	in_output->GetDisplayModeList(in_format, flags, &count, &mode_list[0]);

	for (auto& x : mode_list)
	{
		UINT n = x.RefreshRate.Numerator;
		UINT d = x.RefreshRate.Denominator;
		std::wstring text =
			L"Width = " + std::to_wstring(x.Width) + L" " +
			L"Height = " + std::to_wstring(x.Height) + L" " +
			L"Refresh = " + std::to_wstring(n) + L"/" + std::to_wstring(d) +
			L"\n";

		::OutputDebugString(text.c_str());
	}
}


void Renderer::BuildPipelineState()
{
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc;
    ZeroMemory(&pso_desc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
    
    pso_desc.InputLayout = { m_input_layout.data(), (UINT)m_input_layout.size() };
    pso_desc.pRootSignature = m_root_signature.Get();
    
    pso_desc.VS = 
    { 
        reinterpret_cast<BYTE*>(m_vs_byte_code->GetBufferPointer()), 
        m_vs_byte_code->GetBufferSize() 
    };
    
    pso_desc.PS = 
    { 
        reinterpret_cast<BYTE*>(m_ps_byte_code->GetBufferPointer()), 
        m_ps_byte_code->GetBufferSize() 
    };
    
    pso_desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    pso_desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    pso_desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    pso_desc.SampleMask = UINT_MAX;
    pso_desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    pso_desc.NumRenderTargets = 1;
    pso_desc.RTVFormats[0] = m_back_buffer_format;
    pso_desc.SampleDesc.Count = m_4x_msaa_state ? 4 : 1;
    pso_desc.SampleDesc.Quality = m_4x_msaa_state ? (m_4x_msaa_quality - 1) : 0;
    pso_desc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
    
    AssertHRESULT(m_d3d_device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_pso)));
}