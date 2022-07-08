#pragma once

#include "../Infrastructure/Unique.h"
#include "../Vulkan/Procedure.h"

namespace HyperFast
{
	class PhysicalDevicePicker : public Infra::Unique
	{
	public:
		PhysicalDevicePicker(const VkInstance instance, const Vulkan::InstanceProcedure &instanceProc) noexcept;

		[[nodiscard]]
		VkPhysicalDevice pick() const noexcept;

	private:
		const VkInstance __instance;
		const Vulkan::InstanceProcedure &__instanceProc;

		[[nodiscard]]
		bool __checkVersionSupport(const VkPhysicalDevice physicalDevice) const noexcept;

		[[nodiscard]]
		bool __checkQueueSupport(const VkPhysicalDevice physicalDevice) const noexcept;

		[[nodiscard]]
		uint32_t __getScoreOf(const VkPhysicalDevice physicalDevice) const noexcept;
	};
}
