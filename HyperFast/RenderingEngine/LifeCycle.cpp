#include "LifeCycle.h"

namespace HyperFast
{
	void LifeCycle::tick()
	{
		for (Infra::Event<> &signalEvent : __lifeCycleSignalEvents)
			signalEvent.invoke();
	}
}