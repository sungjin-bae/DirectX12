#include <windows.h>

#include "MyDXApp.h"
#include "RenderObject/RenderObject.h"
#include "RenderObject/RenderObjManager.h"


#if defined(DEBUG) || defined(_DEBUG)
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	MyDXApp m_app(hInstance);
	if (!m_app.Initialize())
		return 0;

	for (int i = 0; i < 100; ++i) {

		auto render_obj = std::make_shared<RenderObject>();
		RenderObjManager::Instance()->Insert(render_obj);
	}
	
	return m_app.Run();
}