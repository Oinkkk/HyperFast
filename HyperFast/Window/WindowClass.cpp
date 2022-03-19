#include "Window.h"
#include "AppInstance.h"
#include "../Infrastructure/StringFormatter.h"
#include <cassert>

namespace Win
{
	WindowClass::WindowClass(const std::string_view &name) :
		__name{ name }, __atom{ __register(name) }
	{}

	WindowClass::~WindowClass() noexcept
	{
		const HINSTANCE hInstance{ AppInstance::getHandle() };
		const BOOL result{ UnregisterClass(__name.c_str(), hInstance) };
		assert(result);
	}

	void WindowClass::loop() noexcept
	{
		MSG msg{};
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	ATOM WindowClass::__register(const std::string_view &name)
	{
		const HINSTANCE hInstance{ AppInstance::getHandle() };

		const WNDCLASS wndClass
		{
			.style = (CS_HREDRAW | CS_VREDRAW),
			.lpfnWndProc = __winProc,
			.cbWndExtra = sizeof(Window *),
			.hInstance = hInstance,
			.lpszClassName = name.data()
		};

		const ATOM retVal{ RegisterClass(&wndClass) };
		if (!retVal)
		{
			const std::string errMsg
			{
				Infra::StringFormatter::format(
					"Cannot register a window class. Error: %d", GetLastError())
			};

			throw std::exception{ errMsg.c_str()};
		}

		return retVal;
	}

	LRESULT WindowClass::__winProc(
		const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) noexcept
	{
		if (uMsg == WM_CREATE)
		{
			CREATESTRUCT *const pCreateStruct{ reinterpret_cast<CREATESTRUCT *>(lParam) };
			const LONG_PTR windowPtrValue{ reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams) };
			SetWindowLongPtr(hwnd, GWLP_USERDATA, windowPtrValue);
		}
		else
		{
			Window *const pWindow{ reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) };
			if (pWindow)
				pWindow->sendRawMessage(uMsg, wParam, lParam);
		}

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}