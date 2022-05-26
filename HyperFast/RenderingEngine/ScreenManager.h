#pragma once

#include "Screen.h"

namespace HyperFast
{
	class ScreenManager : public SurfaceCreater
	{
	public:
		ScreenManager(const VkInstance instance, const VKL::InstanceProcedure &instanceProc) noexcept;
		virtual ~ScreenManager() noexcept = default;

		virtual VkSurfaceKHR create(Win::Window &window) noexcept override;
		virtual void destroy(const VkSurfaceKHR handle) noexcept override;

	private:
		const VkInstance __instance;
		const VKL::InstanceProcedure &__instanceProc;
	};
}
