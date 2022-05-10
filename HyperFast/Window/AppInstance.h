#pragma once

#include <Windows.h>
#include "../Infrastructure/Unique.h"

namespace Win
{
	class AppInstance : public Infra::Unique
	{
	public:
		[[nodiscard]]
		constexpr HINSTANCE getHandle() noexcept;

		static AppInstance &getInstance() noexcept;

	private:
		const HINSTANCE __handle;

		AppInstance() noexcept;
	};

	constexpr HINSTANCE AppInstance::getHandle() noexcept
	{
		return __handle;
	}
}
