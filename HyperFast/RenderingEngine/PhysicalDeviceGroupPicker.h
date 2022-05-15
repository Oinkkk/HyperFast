#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/Procedure.h"
#include <optional>
#include <vector>

namespace HyperFast
{
	class PhysicalDeviceGroupPicker : public Infra::Unique
	{
	public:
		PhysicalDeviceGroupPicker(const VkInstance instance, VKL::InstanceProcedure &instanceProc) noexcept;

		bool pick(VkPhysicalDeviceGroupProperties &physicalDeviceGroupProp) const noexcept;

	private:
		const VkInstance __instance;
		VKL::InstanceProcedure &__instanceProc;

		[[nodiscard]]
		bool __checkVersionSupport(const VkPhysicalDevice physicalDevice) const noexcept;

		[[nodiscard]]
		bool __checkQueueSupport(const VkPhysicalDevice physicalDevice) const noexcept;

		[[nodiscard]]
		uint32_t __getScoreOf(const VkPhysicalDeviceGroupProperties &physicalDeviceGroupProp) const noexcept;
	};
}
