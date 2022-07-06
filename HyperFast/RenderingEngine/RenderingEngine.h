#pragma once

#include <string>
#include "../VulkanLoader/VulkanLoader.h"
#include "../Infrastructure/Logger.h"
#include "ShaderCompiler.h"
#include "../Infrastructure/Environment.h"
#include "Screen.h"
#include "Drawcall.h"

namespace HyperFast
{
	class RenderingEngine final : public Infra::Unique
	{
	public:
		RenderingEngine(Infra::Logger &logger, const std::string_view &appName, const std::string_view &engineName);
		~RenderingEngine() noexcept;

		void startFrame() noexcept;
		void endFrame() noexcept;
		void update() noexcept;

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

		[[nodiscard]]
		std::shared_ptr<Submesh> createSubmesh(const std::shared_ptr<Mesh> &pMesh) noexcept;

		void copyBuffer(
			const VkPipelineStageFlags2 srcStageMask, const VkAccessFlags2 srcAccessMask,
			const VkPipelineStageFlags2 dstStageMask, const VkAccessFlags2 dstAccessMask,
			const VkBuffer dst, const void *const pSrc, const VkDeviceSize srcBufferSize,
			const uint32_t regionCount, const VkBufferCopy *const pRegions,
			const std::any &srcPlaceholder) noexcept;

	private:
		Infra::Logger &__logger;
		const std::string __appName;
		const std::string __engineName;

		uint32_t __instanceVersion{};
		VkDebugUtilsMessengerCreateInfoEXT __debugMessengerCreateInfo{};

		VkInstance __instance{};
		VKL::InstanceProcedure __instanceProc;
		VkDebugUtilsMessengerEXT __debugMessenger{};

		VkPhysicalDevice __physicalDevice{};
		VkPhysicalDeviceProperties2 __physicalDeviceProp2{};
		VkPhysicalDeviceVulkan11Properties __physicalDevice11Prop{};
		VkPhysicalDeviceVulkan12Properties __physicalDevice12Prop{};
		VkPhysicalDeviceVulkan13Properties __physicalDevice13Prop{};

		std::vector<VkQueueFamilyProperties> __queueFamilyProps;
		uint32_t __graphicsQueueFamilyIndex{};

		VkDevice __device{};
		VKL::DeviceProcedure __deviceProc;
		
		VkQueue __graphicsQueue{};

		std::unique_ptr<ScreenManager> __pScreenManager;
		std::unique_ptr<MemoryManager> __pMemoryManager;
		std::unique_ptr<BufferManager> __pBufferManager;

		std::unique_ptr<InstantCommandSubmitter> __pInstantCommandSubmitter;
		std::unique_ptr<BufferCopyManager> __pBufferCopyManager;

		static constexpr inline std::string_view VK_KHRONOS_VALIDATION_LAYER_NAME{ "VK_LAYER_KHRONOS_validation" };

		void __getInstanceVersion() noexcept;
		void __checkInstanceVersionSupport() const;
		void __populateDebugMessengerCreateInfo() noexcept;
		void __createInstance();
		void __destroyInstance() noexcept;
		void __queryInstanceProc() noexcept;
		void __createDebugMessenger();
		void __destroyDebugMessenger() noexcept;
		void __pickPhysicalDevice();
		void __queryPhysicalDeviceProps() noexcept;
		void __retrieveQueueFamilies() noexcept;
		void __createDevice();
		void __destroyDevice() noexcept;
		void __queryDeviceProc() noexcept;
		void __queryGraphicsQueue();

		void __createScreenManager() noexcept;
		void __destroyScreenManager() noexcept;

		void __createMemoryManager() noexcept;
		void __destroyMemoryManager() noexcept;

		void __createBufferManager() noexcept;
		void __destroyBufferManager() noexcept;

		void __createInstantCommandSubmitter() noexcept;
		void __destroyInstantCommandSubmitter() noexcept;

		void __createBufferCopyManager() noexcept;
		void __destroyBufferCopyManager() noexcept;

		void __waitDeviceIdle() const noexcept;

		static VkBool32 VKAPI_PTR vkDebugUtilsMessengerCallbackEXT(
			const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			const VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT *const pCallbackData, void *const pUserData);
	};
}
