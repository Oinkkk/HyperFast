#include "Window.h"
#include "AppInstance.h"
#include "../Infrastructure/StringFormatter.h"
#include <cassert>

namespace Win
{
	Window::Window(
		WindowClass &windowClass, const std::string_view &title, const bool show) :
		Window{ windowClass, title, show, CW_USEDEFAULT, CW_USEDEFAULT }
	{}

	Window::Window(
		WindowClass &windowClass, const std::string_view &title,
		const bool show, const int x, const int y) :
		Window{ windowClass, title, show, x, y, CW_USEDEFAULT, CW_USEDEFAULT }
	{}

	Window::Window(
		WindowClass &windowClass, const std::string_view &title,
		const bool show, const int x, const int y, const int width, const int height) :
		__windowClass{ windowClass }
	{
		__handle = __create(windowClass, title, x, y, width, height, this);
		GetWindowRect(__handle, &__windowRect);
		GetClientRect(__handle, &__clientRect);

		if (show)
			setShow();
	}

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
		ShowWindow(__handle, show ? SW_SHOW : SW_HIDE);
	}

	LRESULT Window::sendRawMessage(const UINT uMsg, const WPARAM wParam, const LPARAM lParam)
	{
		bool customEvent{};

		switch (uMsg)
		{
		case WM_SIZE:
			GetWindowRect(__handle, &__windowRect);
			GetClientRect(__handle, &__clientRect);
			__resizeEvent.invoke(*this, ResizingType(wParam));
			customEvent = true;
			break;

		case WM_PAINT:
			__drawEvent.invoke(*this);
			customEvent = true;
			break;

		case WM_CLOSE:
			destroy();
			customEvent = true;
			break;
		}

		if (customEvent)
			return 0;

		return DefWindowProc(__handle, uMsg, wParam, lParam);
	}

	void Window::setX(const int x) noexcept
	{
		setPosition(x, __windowRect.top);
	}

	void Window::setY(const int y) noexcept
	{
		setPosition(__windowRect.left, y);
	}

	void Window::setPosition(const int x, const int y) noexcept
	{
		__updateAppearance(x, y, __getWindowWidth(), __getWindowHeight());
	}

	void Window::setWidth(const int x) noexcept
	{
		setSize(x, getHeight());
	}

	void Window::setHeight(const int y) noexcept
	{
		setSize(getWidth(), y);
	}

	void Window::setSize(const int width, const int height) noexcept
	{
		const int newWindowWidth{ __getWindowWidth() + (width - getWidth()) };
		const int newWindowHeight{ __getWindowHeight() + (height - getHeight()) };
		__updateAppearance(getX(), getY(), newWindowWidth, newWindowHeight);
	}

	void Window::invalidate() noexcept
	{
		InvalidateRect(__handle, nullptr, FALSE);
	}

	void Window::validate() noexcept
	{
		ValidateRect(__handle, nullptr);
	}

	void Window::destroy()
	{
		if (!__handle)
			return;

		__destroyEvent.invoke(*this);

		const BOOL result{ DestroyWindow(__handle) };
		__handle = nullptr;

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

	void Window::__updateAppearance(const int x, const int y, const int windowWidth, const int windowHeight) noexcept
	{
		MoveWindow(__handle, x, y, windowWidth, windowHeight, FALSE);
		invalidate();
	}

	HWND Window::__create(
		WindowClass &windowClass, const std::string_view &title,
		const int x, const int y, const int width, const int height, Window *const pThis)
	{
		const HINSTANCE hInstance{ AppInstance::getInstance().getHandle() };

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