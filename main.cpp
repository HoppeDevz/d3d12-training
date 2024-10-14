#include <Windows.h>

#include "Window.hpp"
#include "Graphics.hpp"
#include "ApplicationError.hpp"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nCmdShow) {

	try {

		Window* mainWindow = new Window(1280, 720, (LPWSTR)L"D3D12ACADEMY");
		mainWindow->createWindow(hInstance, nCmdShow);

		Graphics* pGraphics = new Graphics(1280, 720, mainWindow->getWindowHandle());
		pGraphics->DisplayAdaptersAndOutputs();
		pGraphics->Init();

		MSG msg = { 0 };

		bool running = true;

		while (running) {

			while (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))
			{

				TranslateMessage(&msg);
				DispatchMessage(&msg);

			}

			pGraphics->Clear();
			pGraphics->Swap();

		}

		return msg.wParam;

	}
	catch (ApplicationError error) {

		OutputDebugStringW(L"erro!");
		Sleep(60 * 1000);
	}
	
}
