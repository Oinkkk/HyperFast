#pragma once

#include "../Infrastructure/Event.h"
#include "../Infrastructure/Timer.h"

namespace Win
{
	class MainLooper
	{
	public:
		void start() noexcept;
		void stop() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<float> &getIdleEvent() noexcept;

		[[nodiscard]]
		static MainLooper &getInstance() noexcept;

	private:
		MainLooper() = default;

		Infra::Timer<> __timer;
		Infra::Event<float> __idleEvent;
	};

	constexpr Infra::EventView<float> &MainLooper::getIdleEvent() noexcept
	{
		return __idleEvent;
	}
}