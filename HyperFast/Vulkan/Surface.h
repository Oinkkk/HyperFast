#pragma once

#include "Instance.h"

namespace Vulkan
{
	class Surface final : public Handle<VkSurfaceKHR>
	{
	public:
		Surface(Instance &instance, const VkWin32SurfaceCreateInfoKHR &createInfo);
		virtual ~Surface() noexcept;

	private:
		Instance &__instance;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkSurfaceKHR __create(
			Instance &instance, const VkWin32SurfaceCreateInfoKHR &createInfo);
	};
}