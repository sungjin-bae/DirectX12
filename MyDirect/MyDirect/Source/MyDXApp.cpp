#include "Common/d3dApp.h"

#include "MyDXApp.h"

MyDXApp::MyDXApp(HINSTANCE in_hInstance) : D3DApp(in_hInstance) {}
MyDXApp::~MyDXApp() {}


bool MyDXApp::Initialize() 
{
	if (!D3DApp::Initialize())
		return false;

	return true;
}

void MyDXApp::OnResize()
{
	D3DApp::OnResize();
}

void MyDXApp::Update(const GameTimer& gt)
{

}

void MyDXApp::Draw(const GameTimer& gt)
{
	// 이전에 실행된 명령 리스트 할당자 재설정.
	AssertHRESULT(mDirectCmdListAlloc->Reset());

	// 재사용을 위한 명령 리스트 초기화.
	AssertHRESULT(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// 백 버퍼 상태 전환
	// D3D12_RESOURCE_STATE_PRESENT -> D3D12_RESOURCE_STATE_RENDER_TARGET
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// 뷰포트 설정
	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// 라이트 블루로 버퍼 백 버퍼 색 초기화
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), DirectX::Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// 깊이-스텔실 버퍼를 출력 병합으로 단계에 설정
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	// 백 버퍼 상태를 출력 준비 상태로 변경
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// 명령어 리스트를 닫고 실행
	AssertHRESULT(mCommandList->Close());

	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// 화면에 출력
	// 백버퍼 인덱스를 업데이트 하여 이중 버퍼 유지.
	AssertHRESULT(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// gpu 완료 대기.
	FlushCommandQueue();
}
