#pragma once

#include <Windows.h>
#include "../Infrastructure/Event.h"

namespace Win
{
	class MainLooper
	{
	public:
		MainLooper() = delete;

		static void start() noexcept;
		static void postQuitMessage() noexcept;
	};
}
