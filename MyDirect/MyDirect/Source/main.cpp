#include <windows.h>

#include "WindowApp.h"
#include "RenderObject/RenderObject.h"
#include "RenderObject/RenderObjManager.h"


#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif


int WINAPI WinMain(HINSTANCE in_h_instance, HINSTANCE in_prev_instance, PSTR in_cmd_line, int in_show_cmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	if (!WindowApp::Instance()->Initialize(in_h_instance))
		return 0;

	return WindowApp::Instance()->Run();
}