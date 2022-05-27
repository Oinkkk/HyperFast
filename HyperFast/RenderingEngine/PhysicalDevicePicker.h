#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/Procedure.h"
#include <optional>
#include <vector>

namespace HyperFast
{
	class PhysicalDevicePicker : public Infra::Unique
	{
	public:
		PhysicalDevicePicker(const VkInstance instance, const VKL::InstanceProcedure &instanceProc) noexcept;

		[[nodiscard]]
		VkPhysicalDevice pick() const noexcept;

	private:
		const VkInstance __instance;
		const VKL::InstanceProcedure &__instanceProc;

		[[nodiscard]]
		bool __checkVersionSupport(const VkPhysicalDevice physicalDevice) const noexcept;

		[[nodiscard]]
		bool __checkQueueSupport(const VkPhysicalDevice physicalDevice) const noexcept;

		[[nodiscard]]
		uint32_t __getScoreOf(const VkPhysicalDevice physicalDevice) const noexcept;
	};
}
