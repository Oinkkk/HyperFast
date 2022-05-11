#pragma once

#include "../Infrastructure/Unique.h"
#include <string>
#include "../VulkanLoader/VulkanLoader.h"

namespace HyperFast
{
	class RenderingEngine final : public Infra::Unique
	{
	public:
		RenderingEngine(const std::string_view &appName, const std::string_view &engineName);
		~RenderingEngine() noexcept;

	private:
		const std::string __appName;
		const std::string __engineName;

		uint32_t __instanceVersion{};
		VkInstance __instance{};
		VKL::InstanceProcedure __instanceProc;

		void __getInstanceVersion() noexcept;
		void __checkInstanceVersionSupport() const;
		void __createInstance();
		void __queryInstanceProc() noexcept;

		void __destroyInstance() noexcept;
		void __resetInstanceProc() noexcept;
	};
}
