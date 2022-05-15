#pragma once

#include "../Infrastructure/Unique.h"
#include <string>
#include "../VulkanLoader/VulkanLoader.h"
#include "../Infrastructure/Logger.h"

namespace HyperFast
{
	class RenderingEngine final : public Infra::Unique
	{
	public:
		RenderingEngine(Infra::Logger &logger, const std::string_view &appName, const std::string_view &engineName);
		~RenderingEngine() noexcept;

	private:
		Infra::Logger &__logger;
		const std::string __appName;
		const std::string __engineName;

		uint32_t __instanceVersion{};
		VkDebugUtilsMessengerCreateInfoEXT __debugMessengerCreateInfo{};

		VkInstance __instance{};
		VKL::InstanceProcedure __instanceProc;
		VkDebugUtilsMessengerEXT __debugMessenger{};

		VkPhysicalDeviceGroupProperties __physicalDeviceGroupProp{};
		VkPhysicalDevice __firstPhysicalDevice{};

		VkPhysicalDeviceProperties2 __physicalDeviceProp2{};
		VkPhysicalDeviceVulkan11Properties __physicalDevice11Prop{};
		VkPhysicalDeviceVulkan12Properties __physicalDevice12Prop{};
		VkPhysicalDeviceVulkan13Properties __physicalDevice13Prop{};

		std::vector<VkQueueFamilyProperties2> __queueFamilyProps;
		std::vector<VkQueueFamilyGlobalPriorityPropertiesKHR> __queueFamilyPriorityProps;
		uint32_t __graphicsQueueFamilyIndex{};
		uint32_t __transferQueueFamilyIndex{};

		static constexpr inline std::string_view VK_KHRONOS_VALIDATION_LAYER_NAME{ "VK_LAYER_KHRONOS_validation" };

		void __getInstanceVersion() noexcept;
		void __checkInstanceVersionSupport() const;
		void __populateDebugMessengerCreateInfo() noexcept;

		void __createInstance();
		void __destroyInstance() noexcept;

		void __queryInstanceProc() noexcept;
		void __resetInstanceProc() noexcept;

		void __createDebugMessenger();
		void __destroyDebugMessenger() noexcept;

		void __pickPhysicalDeviceGroup();
		void __resetPhysicalDeviceGroup() noexcept;

		void __queryPhysicalDeviceProps() noexcept;
		void __resetPhysicalDeviceProps() noexcept;

		void __retrieveQueueFamilies() noexcept;
		void __resetQueueFamilies() noexcept;

		static VkBool32 VKAPI_PTR vkDebugUtilsMessengerCallbackEXT(
			const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			const VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT *const pCallbackData, void *const pUserData);
	};
}
