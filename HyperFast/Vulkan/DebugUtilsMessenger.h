#pragma once

#include "Instance.h"

namespace Vulkan
{
	class DebugUtilsMessenger final : public Handle<VkDebugUtilsMessengerEXT>
	{
	public:
		DebugUtilsMessenger(Instance &instance, const VkDebugUtilsMessengerCreateInfoEXT &createInfo);
		~DebugUtilsMessenger() noexcept;

	private:
		Instance &__instance;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkDebugUtilsMessengerEXT __create(
			Instance &instance, const VkDebugUtilsMessengerCreateInfoEXT &createInfo);
	};
}