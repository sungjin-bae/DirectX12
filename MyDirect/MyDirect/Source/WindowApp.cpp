#include "WindowApp.h"
#include <Windowsx.h>

#include "Camera/CameraManager.h"
#include "Common/Timer.h"
#include "Renderer/Renderer.h"
#include "RenderObject/RenderObjManager.h"
#include "RenderObject/RenderObject.h"

auto CALLBACK MainWndProc(HWND in_hwnd, UINT in_msg, WPARAM in_wParam, LPARAM in_lParam) -> LRESULT
{
    // Forward hwnd on because we can get messages (e.g., WM_CREATE)
    // before CreateWindow returns, and thus before mhMainWnd is valid.
    return WindowApp::Instance()->MsgProc(in_hwnd, in_msg, in_wParam, in_lParam);
}

WindowApp::WindowApp()
    : m_h_app_instance(nullptr), m_h_main_wnd(nullptr), m_app_paused(false), m_minimized(false), m_maximized(false), m_resizing(false), m_fullscreen_state(false), m_main_wnd_caption(L"SJ Direct Engine"), m_client_width(800), m_client_height(600)
{
}

WindowApp::~WindowApp()
{
}

bool WindowApp::Initialize(HINSTANCE in_h_instance)
{
    if (in_h_instance == nullptr)
        return false;

    m_h_app_instance = in_h_instance;

    if (InitMainWindow() == false)
        return false;

    if (InitRenderer() == false)
        return false;

    return true;
}

int WindowApp::Run()
{
    MSG msg = {0};

    m_timer.Reset();

    while (msg.message != WM_QUIT)
    {
        // If there are Window messages then process them.
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        // Otherwise, do animation/game stuff.
        else
        {
            m_timer.Tick();

            if (!m_app_paused)
            {
                Update(m_timer.GetDeltaTime());
                Draw();
            }
            else
            {
                Sleep(100);
            }
        }
    }

    return (int)msg.wParam;
}

void WindowApp::Shutdown()
{
}

bool WindowApp::InitMainWindow()
{
    WNDCLASS wc;
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = m_h_app_instance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
    wc.lpszMenuName = 0;
    wc.lpszClassName = L"MainWnd";

    if (!RegisterClass(&wc))
    {
        MessageBox(0, L"RegisterClass Failed.", 0, 0);
        return false;
    }

    // Compute window rectangle dimensions based on requested client area dimensions.
    RECT rect = {0, 0, m_client_width, m_client_height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, false);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    m_h_main_wnd = CreateWindow(L"MainWnd", m_main_wnd_caption.c_str(),
                                WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, width, height, 0, 0, m_h_app_instance, 0);
    if (!m_h_main_wnd)
    {
        MessageBox(0, L"CreateWindow Failed.", 0, 0);
        return false;
    }

    ShowWindow(m_h_main_wnd, SW_SHOW);
    UpdateWindow(m_h_main_wnd);

    return true;
}

bool WindowApp::InitRenderer()
{
    if (Renderer::Instance()->Init(m_h_main_wnd) == false)
        return false;

    return true;
}

LRESULT WindowApp::MsgProc(HWND in_hwnd, UINT in_msg, WPARAM in_wParam, LPARAM in_lParam)
{
    switch (in_msg)
    {
        // WM_ACTIVATE is sent when the window is activated or deactivated.
        // We pause the game when the window is deactivated and unpause it
        // when it becomes active.
    case WM_ACTIVATE:
        if (LOWORD(in_wParam) == WA_INACTIVE)
        {
            m_app_paused = true;
            m_timer.Stop();
        }
        else
        {
            m_app_paused = false;
            m_timer.Start();
        }
        return 0;

        // WM_SIZE is sent when the user resizes the window.
    case WM_SIZE:
        // Save the new client area dimensions.
        m_client_width = LOWORD(in_lParam);
        m_client_height = HIWORD(in_lParam);
        if (in_wParam == SIZE_MINIMIZED)
        {
            m_app_paused = true;
            m_minimized = true;
            m_maximized = false;
        }
        else if (in_wParam == SIZE_MAXIMIZED)
        {
            m_app_paused = false;
            m_minimized = false;
            m_maximized = true;
            OnResize();
        }
        else if (in_wParam == SIZE_RESTORED)
        {
            // Restoring from minimized state?
            if (m_minimized)
            {
                m_app_paused = false;
                m_minimized = false;
                OnResize();
            }
            // Restoring from maximized state?
            else if (m_maximized)
            {
                m_app_paused = false;
                m_maximized = false;
                OnResize();
            }
            else if (m_resizing)
            {
                // If user is dragging the resize bars, we do not resize
                // the buffers here because as the user continuously
                // drags the resize bars, a stream of WM_SIZE messages are
                // sent to the window, and it would be pointless (and slow)
                // to resize for each WM_SIZE message received from dragging
                // the resize bars.  So instead, we reset after the user is
                // done resizing the window and releases the resize bars, which
                // sends a WM_EXITSIZEMOVE message.
            }
            else // API call such as SetWindowPos or mSwapChain->SetFullscreenState.
            {
                OnResize();
            }
        }
        return 0;

        // WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
    case WM_ENTERSIZEMOVE:
        m_app_paused = true;
        m_resizing = true;
        m_timer.Stop();
        return 0;

        // WM_EXITSIZEMOVE is sent when the user releases the resize bars.
        // Here we reset everything based on the new window dimensions.
    case WM_EXITSIZEMOVE:
        m_app_paused = false;
        m_resizing = false;
        m_timer.Start();
        OnResize();
        return 0;

        // WM_DESTROY is sent when the window is being destroyed.
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

        // The WM_MENUCHAR message is sent when a menu is active and the user presses
        // a key that does not correspond to any mnemonic or accelerator key.
    case WM_MENUCHAR:
        // Don't beep when we alt-enter.
        return MAKELRESULT(0, MNC_CLOSE);

        // Catch this message so to prevent the window from becoming too small.
    case WM_GETMINMAXINFO:
        ((MINMAXINFO*)in_lParam)->ptMinTrackSize.x = 200;
        ((MINMAXINFO*)in_lParam)->ptMinTrackSize.y = 200;
        return 0;

    case WM_LBUTTONDOWN:
    case WM_MBUTTONDOWN:
    case WM_RBUTTONDOWN:
        OnMouseDown(in_wParam, GET_X_LPARAM(in_lParam), GET_Y_LPARAM(in_lParam));
        return 0;
    case WM_LBUTTONUP:
    case WM_MBUTTONUP:
    case WM_RBUTTONUP:
        OnMouseUp(in_wParam, GET_X_LPARAM(in_lParam), GET_Y_LPARAM(in_lParam));
        return 0;
    case WM_MOUSEMOVE:
        OnMouseMove(in_wParam, GET_X_LPARAM(in_lParam), GET_Y_LPARAM(in_lParam));
        return 0;
    case WM_KEYDOWN:
        OnKeyDown(in_wParam);
        return 0;
    case WM_KEYUP:
        OnKeyUp(in_wParam);
        return 0;
    }

    return DefWindowProc(in_hwnd, in_msg, in_wParam, in_lParam);
}

void WindowApp::OnResize()
{
}

void WindowApp::Update(float in_deltaTime)
{
}

void WindowApp::Draw()
{
    for (uint8_t stack_id = 0; stack_id < static_cast<uint8_t>(CameraManager::EStackID::MAX) + 1; ++stack_id)
    {
        for (const auto &[camera_order, camera_ptr] : CameraManager::Instance()->GetCameras(static_cast<CameraManager::EStackID>(stack_id)))
        {
            camera_ptr->UpdateViewMatrix();

            // 모든 렌더러 타입에 대해 체크
            for (uint8_t type = 0; type < static_cast<uint8_t>(ERendererType::MAX); ++type)
            {
                if (camera_ptr->GetRendererTypeBit(static_cast<ERendererType>(type)))
                {
                    const auto& render_objects = RenderObjManager::Instance()->GetRenderObjects(static_cast<ERendererType>(type));
                    for (const auto& render_object : render_objects)
                        render_object->Update();
                }

                // 뷰 포지션 업데이트 완료.
                // render_object 의 버텍스 정보를 세팅해야한다.
            }
        }
    }
}

void WindowApp::OnMouseDown(WPARAM in_btnState, int in_x, int in_y)
{
}

void WindowApp::OnMouseUp(WPARAM in_btnState, int in_x, int in_y)
{
}

void WindowApp::OnMouseMove(WPARAM in_btnState, int in_x, int in_y)
{
}

void WindowApp::OnKeyDown(WPARAM in_wParam)
{
}

void WindowApp::OnKeyUp(WPARAM in_wParam)
{
}
