#include "MainLooper.h"
#include <Windows.h>

namespace Win
{
	void MainLooper::start() noexcept
	{
		MSG msg{};
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	void MainLooper::postQuitMessage() noexcept
	{
		PostQuitMessage(0);
	}
}