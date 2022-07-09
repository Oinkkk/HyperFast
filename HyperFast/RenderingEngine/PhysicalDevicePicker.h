#pragma once

#include "../Vulkan/Instance.h"

namespace HyperFast
{
	class PhysicalDevicePicker : public Infra::Unique
	{
	public:
		PhysicalDevicePicker(Vulkan::Instance &instance) noexcept;

		[[nodiscard]]
		VkPhysicalDevice pick() const;

	private:
		Vulkan::Instance &__instance;

		[[nodiscard]]
		bool __checkVersionSupport(const VkPhysicalDevice physicalDevice) const noexcept;

		[[nodiscard]]
		bool __checkQueueSupport(const VkPhysicalDevice physicalDevice) const noexcept;

		[[nodiscard]]
		uint32_t __getScoreOf(const VkPhysicalDevice physicalDevice) const noexcept;
	};
}
