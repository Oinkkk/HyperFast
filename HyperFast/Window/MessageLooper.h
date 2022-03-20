#pragma once

#include <Windows.h>
#include "../Infrastructure/Event.h"

namespace Win
{
	class MessageLooper
	{
	public:
		MessageLooper() = delete;

		static void startLoop() noexcept;
		static void stopLoop() noexcept;
	};
}
