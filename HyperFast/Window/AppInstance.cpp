#include "AppInstance.h"

namespace Win
{
	AppInstance::AppInstance() noexcept :
		__handle{ GetModuleHandle(nullptr) }
	{}

	HINSTANCE AppInstance::getHandle() noexcept
	{
		return __getInstance().__handle;
	}

	AppInstance &AppInstance::__getInstance() noexcept
	{
		static AppInstance instance;
		return instance;
	}
}