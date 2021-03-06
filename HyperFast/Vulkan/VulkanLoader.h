#pragma once

#include "../Infrastructure/Unique.h"
#include "Procedure.h"
#include <Windows.h>

namespace Vulkan
{
	class VulkanLoader : public Infra::Unique
	{
	public:
		void load();
		void free();

		[[nodiscard]]
		constexpr const GlobalProcedure &getGlobalProcedure() const noexcept;

		[[nodiscard]]
		InstanceProcedure queryInstanceProcedure(const VkInstance instance) noexcept;

		[[nodiscard]]
		DeviceProcedure queryDeviceProcedure(
			const PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr, const VkDevice device) noexcept;

		[[nodiscard]]
		static VulkanLoader &getInstance() noexcept;

	private:
		HMODULE __handle{};
		GlobalProcedure __globalProc;

		VulkanLoader() = default;
		~VulkanLoader();

		void __loadGlobalProc(const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) noexcept;
	};

	constexpr const GlobalProcedure &VulkanLoader::getGlobalProcedure() const noexcept
	{
		return __globalProc;
	}
}