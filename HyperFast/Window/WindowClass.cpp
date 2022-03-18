#include "WindowClass.h"
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

	ATOM WindowClass::__register(const std::string_view &name)
	{
		const HINSTANCE hInstance{ AppInstance::getHandle() };

		WNDCLASS wndClass
		{
			.lpfnWndProc = __winProc,
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
		const HWND hwnd, const UINT message, const WPARAM wparam, const LPARAM lparam) noexcept
	{
		const HINSTANCE hInstance{ AppInstance::getHandle() };

		return {};
	}
}