#include "MessageLooper.h"
#include <Windows.h>

namespace Win
{
	void MessageLooper::startLoop() noexcept
	{
		MSG msg{};
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void MessageLooper::stopLoop() noexcept
	{
		PostQuitMessage(0);
	}
}