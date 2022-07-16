#pragma once

#include <string>
#include "../Infrastructure/Logger.h"
#include "../Vulkan/Queue.h"
#include "ShaderCompiler.h"
#include "../Infrastructure/Environment.h"
#include "Screen.h"
#include "Drawcall.h"
#include "../Vulkan/DebugUtilsMessenger.h"
#include "CommandSubmitter.h"

namespace HyperFast
{
	class RenderingEngine final : public Infra::Unique
	{
	public:
		RenderingEngine(Infra::Logger &logger, const std::string_view &appName, const std::string_view &engineName);
		~RenderingEngine() noexcept;

		[[nodiscard]]
		std::shared_ptr<Screen> createScreen(Win::Window &window);

		[[nodiscard]]
		std::shared_ptr<Buffer> createBuffer(
			const VkDeviceSize size, const VkBufferUsageFlags usage);

		[[nodiscard]]
		std::shared_ptr<Memory> createMemory(
			const VkMemoryRequirements &memRequirements,
			const VkMemoryPropertyFlags requiredProps, const bool linearity);

		[[nodiscard]]
		std::unique_ptr<Drawcall> createDrawcall() noexcept;

		[[nodiscard]]
		std::shared_ptr<Mesh> createMesh() noexcept;

		void enqueueCommands(
			const SubmitLayerType layerType,
			const uint32_t waitSemaphoreInfoCount,
			const VkSemaphoreSubmitInfo *const pWaitSemaphoreInfos,
			const uint32_t commandBufferInfoCount,
			const VkCommandBufferSubmitInfo *const pCommandBufferInfos,
			const uint32_t signalSemaphoreInfoCount,
			const VkSemaphoreSubmitInfo *const pSignalSemaphoreInfos) noexcept;

		void submit();

	private:
		Infra::Logger &__logger;
		const std::string __appName;
		const std::string __engineName;

		uint32_t __instanceVersion{};
		VkDebugUtilsMessengerCreateInfoEXT __debugMessengerCreateInfo{};

		std::unique_ptr<Vulkan::Instance> __pInstance;
		std::unique_ptr<Vulkan::DebugUtilsMessenger> __pDebugMessenger;

		std::unique_ptr<Vulkan::PhysicalDevice> __pPhysicalDevice;
		VkPhysicalDeviceProperties2 __physicalDeviceProp2{};
		VkPhysicalDeviceVulkan11Properties __physicalDevice11Prop{};
		VkPhysicalDeviceVulkan12Properties __physicalDevice12Prop{};
		VkPhysicalDeviceVulkan13Properties __physicalDevice13Prop{};

		uint32_t __queueFamilyIndex{};
		std::unique_ptr<Vulkan::Device> __pDevice;
		std::unique_ptr<Vulkan::Queue> __pQueue;

		std::unique_ptr<ScreenManager> __pScreenManager;
		std::unique_ptr<MemoryManager> __pMemoryManager;
		std::unique_ptr<BufferManager> __pBufferManager;
		std::unique_ptr<CommandSubmitter> __pCommandSubmitter;

		static constexpr inline std::string_view VK_KHRONOS_VALIDATION_LAYER_NAME{ "VK_LAYER_KHRONOS_validation" };

		void __getInstanceVersion() noexcept;
		void __checkInstanceVersionSupport() const;
		void __populateDebugMessengerCreateInfo() noexcept;
		void __createInstance();
		void __createDebugMessenger();
		void __pickPhysicalDevice();
		void __queryPhysicalDeviceProps() noexcept;
		void __pickGraphicsQueueFamily() noexcept;
		void __createDevice();
		void __makeQueue() noexcept;

		void __createScreenManager() noexcept;
		void __createMemoryManager() noexcept;
		void __createBufferManager() noexcept;
		void __createCommandSubmitter() noexcept;

		static VkBool32 VKAPI_PTR vkDebugUtilsMessengerCallbackEXT(
			const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			const VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT *const pCallbackData, void *const pUserData);
	};
}