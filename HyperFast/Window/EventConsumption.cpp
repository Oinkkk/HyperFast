#include "EventConsumption.h"

namespace Win
{
	void EventConsumption::consume() noexcept
	{
		__consumed |= true;
	}

	bool EventConsumption::isConsumed() const noexcept
	{
		return __consumed;
	}
}