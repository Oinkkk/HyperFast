#pragma once

#include "ScreenManager.h"

namespace HyperFast
{
	class Screen final : public Infra::Unique
	{
	public:
		Screen(ScreenManager &screenManager, Win::Window &window);
		~Screen() noexcept = default;

		void setDrawcall(Drawcall *const pDrawcall) noexcept;
		void render();
		void present() noexcept;

	private:
		ScreenManager &__screenManager;
		std::unique_ptr<ScreenManager::ScreenImpl> __pImpl;
	};
}
