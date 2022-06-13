#include "MainLooper.h"
#include <Windows.h>

namespace Win
{
	void MainLooper::start() noexcept
	{
		MSG msg{};
		while (true)
		{
			if (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					break;

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				// IDLE
				__timer.end();
				const float elapsedTime{ __timer.getElapsed() };
				__timer.start();

				__idleEvent.invoke(elapsedTime);
			}
		}
	}

	void MainLooper::stop() noexcept
	{
		PostQuitMessage(0);
	}

	MainLooper &MainLooper::getInstance() noexcept
	{
		static MainLooper instance;
		return instance;
	}
}