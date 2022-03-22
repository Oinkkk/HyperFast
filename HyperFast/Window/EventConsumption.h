#pragma once

#include "../Infrastructure/Unique.h"

namespace Win
{
	class EventConsumption : public Infra::Unique
	{
	public:
		virtual void consume() noexcept = 0;

		[[nodiscard]]
		virtual bool isConsumed() const noexcept = 0;
	};

	class EventConsumptionImpl : public EventConsumption
	{
	public:
		virtual void consume() noexcept override;

		[[nodiscard]]
		virtual bool isConsumed() const noexcept override;

		constexpr void reset() noexcept;

	private:
		bool __consumed{};
	};

	constexpr void EventConsumptionImpl::reset() noexcept
	{
		__consumed = false;
	}
}
