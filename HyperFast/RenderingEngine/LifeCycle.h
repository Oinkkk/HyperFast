#pragma once

#include "../Infrastructure/Event.h"
#include "LifeCycleType.h"

namespace HyperFast
{
	class LifeCycle : public Infra::Unique
	{
	public:
		[[nodiscard]]
		constexpr Infra::EventView<> &getSignalEvent(const LifeCycleType signalType) noexcept;

		void tick();

	private:

		Infra::Event<> __lifeCycleSignalEvents[NUM_LIFE_CYCLE_TYPES];
	};

	constexpr Infra::EventView<> &LifeCycle::getSignalEvent(const LifeCycleType signalType) noexcept
	{
		return __lifeCycleSignalEvents[uint32_t(signalType) - 1U];
	}
}
