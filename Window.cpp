
#include "Window.hpp"
#include "ApplicationError.hpp"

Window::Window(int _width, int _height, LPWSTR _windowName) {

	this->windowName = _windowName;

	this->wc = { 0 };
	this->windowHandle = 0;

	this->width = _width;
	this->height = _height;

}

void Window::createWindow(HINSTANCE hInstance, int nCmdShow) {

	this->wc.cbSize = sizeof(WNDCLASSEX);
	this->wc.style = CS_VREDRAW | CS_HREDRAW;
	this->wc.cbClsExtra = 0;
	this->wc.cbWndExtra = 0;
	this->wc.lpfnWndProc = this->WinProc;
	this->wc.hInstance = hInstance;
	this->wc.lpszClassName = this->windowName;
	this->wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 122, 204));
	this->wc.lpszMenuName = nullptr;
	this->wc.hCursor = LoadCursorW(NULL, IDC_ARROW);
	this->wc.hIcon = LoadIconW(NULL, IDI_APPLICATION);
	this->wc.hIconSm = LoadIconW(NULL, IDI_APPLICATION);

	if (!RegisterClassExW(&this->wc)) {
		ThrowAppError();
	}

	UINT dpi = GetDpiForSystem();
	UINT sWidth = GetSystemMetricsForDpi(SM_CXSCREEN, dpi);
	UINT sHeight = GetSystemMetricsForDpi(SM_CYSCREEN, dpi);

	this->windowHandle = CreateWindowW(
		this->windowName,
		this->windowName,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		sWidth / 2 - this->width / 2,
		sHeight / 2 - this->height / 2,
		this->width,
		this->height,
		nullptr,
		nullptr,
		hInstance,
		nullptr
	);

	ShowWindow(this->windowHandle, nCmdShow);

}

HWND Window::getWindowHandle() {

	return this->windowHandle;

}

LRESULT CALLBACK Window::WinProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {

	switch (msg) {

		case WM_DESTROY: {

			PostQuitMessage(0);
			return 0;
		}

	}

	return DefWindowProcW(hwnd, msg, wParam, lParam);

}