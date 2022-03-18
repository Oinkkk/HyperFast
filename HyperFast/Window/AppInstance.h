#pragma once

#include <Windows.h>
#include "../Infrastructure/Unique.h"

namespace Win
{
	class AppInstance : public Infra::Unique
	{
	public:
		static HINSTANCE getHandle() noexcept;

	private:
		const HINSTANCE __handle;

		AppInstance() noexcept;
		static AppInstance &__getInstance() noexcept;
	};
}
