#include "AppInstance.h"

namespace Win
{
	AppInstance::AppInstance() noexcept :
		__handle{ GetModuleHandle(nullptr) }
	{}

	AppInstance &AppInstance::getInstance() noexcept
	{
		static AppInstance instance;
		return instance;
	}
}