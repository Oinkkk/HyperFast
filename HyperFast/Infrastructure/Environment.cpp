#include "Environment.h"

namespace Infra
{
	Environment &Environment::getInstance() noexcept
	{
		static Environment instance;
		return instance;
	}
}