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
		try
		{
			destroy();
		}
		catch (...)
		{ }
	}

	void Window::setShow(const bool show) noexcept
	{
		if (show)
			ShowWindow(__handle, SW_SHOW);
		else
			ShowWindow(__handle, SW_HIDE);
	}

	LRESULT Window::sendRawMessage(const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
	{
		switch (uMsg)
		{
		case WM_CLOSE:
			__closeEvent.invoke(this);
			break;

		case WM_PAINT:
			ValidateRect(__handle, nullptr);
			break;

		default:
			return DefWindowProc(__handle, uMsg, wParam, lParam);
		}

		return 0;
	}

	void Window::destroy()
	{
		if (__destroyed)
			return;

		const BOOL result{ DestroyWindow(__handle) };
		__destroyed = true;

		if (!result)
		{
			const std::string errMsg
			{
				Infra::StringFormatter::format(
					"Error occurred while destroying the window. Error: %d", GetLastError())
			};

			throw std::exception{ errMsg.c_str() };
		}
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
				pThis
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