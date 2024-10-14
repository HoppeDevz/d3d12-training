#pragma once

#include <Windows.h>

class Window {

	private:
		WNDCLASSEX wc;

		HWND windowHandle;
		LPWSTR windowName;

		int width;
		int height;

	public:
			
		Window(int _width, int _height, const LPWSTR _windowName);

		void createWindow(HINSTANCE hInstance, int nCmdShow);
		HWND getWindowHandle();

		static LRESULT CALLBACK WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

};
