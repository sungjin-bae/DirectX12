#pragma once

#include <Windows.h>
#include <string>
#include "Common/Singleton.h"
#include "Common/Timer.h"

class WindowApp : public Singleton<WindowApp>
{
public:
    WindowApp();
    virtual ~WindowApp();

    bool Initialize(HINSTANCE in_hInstance);
    int Run();
    void Shutdown();

    // Window dimensions
    int GetClientWidth() const { return m_client_width; }
    int GetClientHeight() const { return m_client_height; }
    HWND GetMainWnd() const { return m_h_main_wnd; }

    // Window message handlers
    virtual LRESULT MsgProc(HWND in_hwnd, UINT in_msg, WPARAM in_wParam, LPARAM in_lParam);

protected:
    // Override these functions in derived class
    virtual bool Init() { return true; }
    virtual void OnResize() {}
    virtual void Update(float in_deltaTime) {}
    virtual void Draw() {}
    virtual void OnMouseDown(WPARAM in_btnState, int in_x, int in_y) {}
    virtual void OnMouseUp(WPARAM in_btnState, int in_x, int in_y) {}
    virtual void OnMouseMove(WPARAM in_btnState, int in_x, int in_y) {}
    virtual void OnKeyDown(WPARAM in_wParam) {}
    virtual void OnKeyUp(WPARAM in_wParam) {}

    // Window creation
    bool InitMainWindow();

private:
    HINSTANCE m_h_app_instance;        // application instance handle
    HWND      m_h_main_wnd;        // main window handle
    bool      m_app_paused;       // is the application paused?
    bool      m_minimized;       // is the application minimized?
    bool      m_maximized;       // is the application maximized?
    bool      m_resizing;        // are the resize bars being dragged?
    bool      m_fullscreen_state; // fullscreen enabled

    // Derived class should set these in derived constructor to customize starting values.
    std::wstring m_main_wnd_caption;
    int m_client_width;
    int m_client_height;

    Timer m_timer;
};
