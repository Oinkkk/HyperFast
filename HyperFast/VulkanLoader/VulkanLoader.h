#pragma once

#include "../Infrastructure/Unique.h"
#include "Procedure.h"

namespace VKL
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

		static VulkanLoader &getInstance() noexcept;

	private:
		HMODULE __handle{};
		GlobalProcedure __globalProc;
		PFN_vkGetInstanceProcAddr __global_vkGetInstanceProcAddr{};

		VulkanLoader() = default;
		~VulkanLoader();

		void __loadGlobalProc() noexcept;
	};

	constexpr const GlobalProcedure &VulkanLoader::getGlobalProcedure() const noexcept
	{
		return __globalProc;
	}
}