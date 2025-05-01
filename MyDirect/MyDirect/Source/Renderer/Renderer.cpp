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
	BuildDescriptorHeaps();
	BuildConstantBuffers();
	BuildRootSignature();

    // 뷰포트와 가위 영역을 초기화합니다.
    m_screen_viewport.TopLeftX = 0;
    m_screen_viewport.TopLeftY = 0;
    m_screen_viewport.Width = static_cast<float>(m_client_width);
    m_screen_viewport.Height = static_cast<float>(m_client_height);
    m_screen_viewport.MinDepth = 0.0f;
    m_screen_viewport.MaxDepth = 1.0f;

    m_scissor_rect = { 0, 0, m_client_width, m_client_height };

    // 임시 RenderObject를 생성하여 PSO를 초기화합니다.
    auto temp_obj = std::make_shared<RenderObject>(XMFLOAT3(0, 0, 0), XMFLOAT3(0, 0, 0), XMFLOAT3(1, 1, 1));
    BuildPSO(temp_obj);

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

    m_command_list->RSSetViewports(1, &m_screen_viewport);
    m_command_list->RSSetScissorRects(1, &m_scissor_rect);

    // 리소스 사용 상태 전환을 표시합니다.
    m_command_list->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    // 백 버퍼와 깊이 버퍼를 지웁니다.
    m_command_list->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
    m_command_list->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    // 렌더링할 버퍼를 지정합니다.
    m_command_list->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

    ID3D12DescriptorHeap* descriptor_heaps[] = { m_cbv_heap.Get() };
    m_command_list->SetDescriptorHeaps(_countof(descriptor_heaps), descriptor_heaps);

    m_command_list->SetGraphicsRootSignature(m_root_signature.Get());

    // 명령어 기록을 마칩니다.
    AssertHRESULT(m_command_list->Close());

    // 명령어 리스트를 큐에 추가하여 실행합니다.
    ID3D12CommandList* cmd_lists[] = { m_command_list.Get() };
    m_command_queue->ExecuteCommandLists(_countof(cmd_lists), cmd_lists);

    // 백 버퍼와 프론트 버퍼를 교체합니다.
    AssertHRESULT(m_swap_chain->Present(0, 0));
    m_curr_back_buffer = (m_curr_back_buffer + 1) % swap_chain_buffer_count;

    // 프레임 명령어가 완료될 때까지 기다립니다.
    FlushCommandQueue();
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


void Renderer::BuildDescriptorHeaps()
{
    D3D12_DESCRIPTOR_HEAP_DESC cbv_heap_desc;

    // 힙에 포함될 디스크립터의 개수를 지정합니다.
    cbv_heap_desc.NumDescriptors = 1;

    // CBV(Constant Buffer View), SRV(Shader Resource View), UAV(Unordered Access View)를 저장하는 힙 타입을 지정합니다.
    cbv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

    // 셰이더에서 이 힙의 디스크립터에 접근할 수 있도록 설정합니다.
	// 이 플래그가 없으면 CPU 전용 힙이 되어 셰이더가 사용할 수 없음.
    cbv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    // 멀티 GPU 시스템에서 사용할 GPU 노드를 지정합니다. 0은 단일 GPU 시스템을 의미합니다.
    cbv_heap_desc.NodeMask = 0;

    AssertHRESULT(m_d3d_device->CreateDescriptorHeap(&cbv_heap_desc,
        IID_PPV_ARGS(&m_cbv_heap)));
}

void Renderer::BuildPSO(const RenderObjSharedPtr& in_render_object)
{
	if (in_render_object == nullptr)
		return;

    // RenderObject에서 PSO 설정을 가져옵니다.
    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = in_render_object->GetPSODesc();
    
    // 루트 시그니처 설정
    pso_desc.pRootSignature = m_root_signature.Get();
    
    AssertHRESULT(m_d3d_device->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&m_pso)));
}

void Renderer::BuildConstantBuffers()
{
    // 상수 버퍼 리소스를 생성합니다.
    m_object_constants_buffer = std::make_unique<UploadBuffer<ObjectConstants>>(m_d3d_device.Get(), 1, true);
    
    // 상수 버퍼의 크기를 계산합니다.
    UINT obj_cb_byte_size = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    // GPU 가상 주소를 가져옵니다.
    D3D12_GPU_VIRTUAL_ADDRESS cb_address = m_object_constants_buffer->Resource()->GetGPUVirtualAddress();
    
    // 첫 번째 오브젝트의 상수 버퍼 오프셋을 계산합니다.
    int box_cbuf_index = 0;
    cb_address += box_cbuf_index * obj_cb_byte_size;

    // 상수 버퍼 뷰를 생성합니다.
    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
    cbv_desc.BufferLocation = cb_address;
    cbv_desc.SizeInBytes = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));

    // 디스크립터 힙에 상수 버퍼 뷰를 생성합니다.
    m_d3d_device->CreateConstantBufferView(
        &cbv_desc,
        m_cbv_heap->GetCPUDescriptorHandleForHeapStart());
}

void Renderer::BuildRootSignature()
{
    // 셰이더 프로그램은 일반적으로 입력으로 리소스(상수 버퍼, 텍스처, 샘플러 등)가 필요합니다.
    // 루트 시그니처는 셰이더 프로그램이 기대하는 리소스를 정의합니다.
    // 셰이더 프로그램을 함수로 생각하고, 입력 리소스를 함수 매개변수로 생각한다면,
    // 루트 시그니처는 함수 시그니처를 정의하는 것과 같습니다.

    // 루트 파라미터는 테이블, 루트 디스크립터 또는 루트 상수가 될 수 있습니다.
    CD3DX12_ROOT_PARAMETER slot_root_parameter[1];

    // CBV의 단일 디스크립터 테이블을 생성합니다.
    CD3DX12_DESCRIPTOR_RANGE cbv_table;
    cbv_table.Init(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, 1, 0);
    slot_root_parameter[0].InitAsDescriptorTable(1, &cbv_table);

    // 루트 시그니처는 루트 파라미터의 배열입니다.
    CD3DX12_ROOT_SIGNATURE_DESC root_sig_desc(1, slot_root_parameter, 0, nullptr,
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    // 단일 슬롯을 가진 루트 시그니처를 생성합니다.
    // 이 슬롯은 단일 상수 버퍼로 구성된 디스크립터 범위를 가리킵니다.
    Microsoft::WRL::ComPtr<ID3DBlob> serialized_root_sig = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> error_blob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&root_sig_desc, D3D_ROOT_SIGNATURE_VERSION_1,
        serialized_root_sig.GetAddressOf(), error_blob.GetAddressOf());

    if (error_blob != nullptr)
    {
        ::OutputDebugStringA((char*)error_blob->GetBufferPointer());
    }
    AssertHRESULT(hr);

    AssertHRESULT(m_d3d_device->CreateRootSignature(
        0,
        serialized_root_sig->GetBufferPointer(),
        serialized_root_sig->GetBufferSize(),
        IID_PPV_ARGS(&m_root_signature)));
}

ID3D12Resource* Renderer::CurrentBackBuffer() const
{
    return m_swap_chain_buffer[m_curr_back_buffer].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::CurrentBackBufferView() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        m_rtv_heap->GetCPUDescriptorHandleForHeapStart(),
        m_curr_back_buffer,
        m_rtv_descriptor_size);
}

D3D12_CPU_DESCRIPTOR_HANDLE Renderer::DepthStencilView() const
{
    return m_dsv_heap->GetCPUDescriptorHandleForHeapStart();
}

void Renderer::FlushCommandQueue()
{
    // 현재 펜스 값에 1을 더합니다.
    m_current_fence++;

    // 명령어 큐에 펜스 명령어를 추가합니다.
    AssertHRESULT(m_command_queue->Signal(m_fence.Get(), m_current_fence));

    // GPU가 펜스 지점에 도달할 때까지 기다립니다.
    if (m_fence->GetCompletedValue() < m_current_fence)
    {
        HANDLE event_handle = CreateEventEx(nullptr, nullptr, false, EVENT_ALL_ACCESS);

        // 펜스 이벤트를 설정합니다.
        AssertHRESULT(m_fence->SetEventOnCompletion(m_current_fence, event_handle));

        // 이벤트가 발생할 때까지 기다립니다.
        WaitForSingleObject(event_handle, INFINITE);
        CloseHandle(event_handle);
    }
}


