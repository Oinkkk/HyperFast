#include "EventConsumption.h"

namespace Win
{
	void EventConsumptionImpl::consume() noexcept
	{
		__consumed |= true;
	}

	bool EventConsumptionImpl::isConsumed() const noexcept
	{
		return __consumed;
	}
}