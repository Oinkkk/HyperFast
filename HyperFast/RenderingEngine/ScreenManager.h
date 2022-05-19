#pragma once

#include "Screen.h"
#include "../Window/Window.h"

namespace HyperFast
{
	class ScreenManager : public SurfaceAllocator
	{
	public:
		ScreenManager() noexcept;
		~ScreenManager() noexcept;

		virtual VkSurfaceKHR allocate(Win::Window &window) override;
		virtual void free(const VkSurfaceKHR handle) noexcept override;

	private:

	};
}
