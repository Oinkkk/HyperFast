#include "Window.h"
#include "AppInstance.h"
#include "../Infrastructure/StringFormatter.h"
#include <cassert>

namespace Win
{
	WindowClass::WindowClass(const HINSTANCE hInstance, const std::string_view &name) :
		__hInstance{ hInstance }, __name { name }, __atom{ __register(hInstance, name) }
	{}

	WindowClass::~WindowClass() noexcept
	{
		const BOOL result{ UnregisterClass(__name.c_str(), __hInstance) };
		assert(result);
	}

	ATOM WindowClass::__register(const HINSTANCE hInstance, const std::string_view &name)
	{
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

		Window *const pWindow{ reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA)) };
		if (pWindow)
			return pWindow->sendRawMessage(uMsg, wParam, lParam);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
}