#include "Window.h"
#include "AppInstance.h"
#include "../Infrastructure/StringFormatter.h"
#include <cassert>

namespace Win
{
	Window::Window(const WindowClass &windowClass, const std::string_view &title) :
		Window{ windowClass, title, CW_USEDEFAULT, CW_USEDEFAULT }
	{}

	Window::Window(
		const WindowClass &windowClass, const std::string_view &title,
		const int x, const int y) :
		Window{ windowClass, title, x, y, CW_USEDEFAULT, CW_USEDEFAULT }
	{}

	Window::Window(
		const WindowClass &windowClass, const std::string_view &title,
		const int x, const int y, const int width, const int height) :
		__handle{ __create(windowClass, title, x, y, width, height, this) }
	{}

	Window::~Window() noexcept
	{
		const BOOL result{ DestroyWindow(__handle) };
		assert(result);
	}

	void Window::setShow(const bool show) noexcept
	{
		if (show)
			ShowWindow(__handle, SW_SHOW);
		else
			ShowWindow(__handle, SW_HIDE);
	}

	HWND Window::__create(
		const WindowClass &windowClass, const std::string_view &title,
		const int x, const int y, const int width, const int height, Window *const pThis)
	{
		const HINSTANCE hInstance{ AppInstance::getHandle() };

		const HWND retVal
		{
			CreateWindowEx
			(
				// dwExStyle
				0,

				// lpClassName
				windowClass.getName().c_str(),

				// lpWindowName
				title.data(),
				
				// dwStyle (Window style)
				WS_OVERLAPPEDWINDOW,

				// position & size
				x, y, width, height,
				
				// hWndParent (Parent window)
				nullptr,
				
				// hMenu
				nullptr,
				
				// hInstance (Instance handle)
				hInstance,
				
				// lpParam (Additional application data)
				nullptr
			)
		};

		if (!retVal)
		{
			const std::string errMsg
			{
				Infra::StringFormatter::format(
					"Cannot create a window. Error: %d", GetLastError())
			};

			throw std::exception{ errMsg.c_str() };
		}

		return retVal;
	}
}