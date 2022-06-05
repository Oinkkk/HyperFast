#pragma once

#include "ScreenManager.h"

namespace HyperFast
{
	class Screen final : public Infra::Unique
	{
	public:
		Screen(ScreenManager &screenManager, Win::Window &window);
		~Screen() noexcept = default;

		bool draw() noexcept;

	private:
		ScreenManager &__screenManager;
		std::unique_ptr<ScreenManager::ScreenImpl> __pImpl;
	};
}
