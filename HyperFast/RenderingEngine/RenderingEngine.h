#pragma once

#include <string>
#include "../Infrastructure/Logger.h"
#include "../Vulkan/Queue.h"
#include "ShaderCompiler.h"
#include "../Infrastructure/Environment.h"
#include "Screen.h"
#include "../Vulkan/DebugUtilsMessenger.h"
#include "InstantCommandExecutor.h"
#include "LifeCycle.h"

namespace HyperFast
{
	class RenderingEngine final : public Infra::Unique
	{
	public:
		RenderingEngine(Infra::Logger &logger, const std::string_view &appName, const std::string_view &engineName);
		~RenderingEngine() noexcept;

		[[nodiscard]]
		constexpr LifeCycle &getLifeCycle() noexcept;

		[[nodiscard]]
		std::shared_ptr<Screen> createScreen(Win::Window &window);

		[[nodiscard]]
		std::unique_ptr<Buffer> createBuffer(
			const VkDeviceSize size, const VkBufferUsageFlags usage);

		[[nodiscard]]
		std::shared_ptr<Memory> createMemory(
			const VkMemoryRequirements &memRequirements,
			const VkMemoryPropertyFlags requiredProps, const bool linearity);

		[[nodiscard]]
		std::unique_ptr<Drawcall> createDrawcall() noexcept;

		[[nodiscard]]
		std::shared_ptr<Mesh> createMesh() noexcept;

		void tick();

	private:
		Infra::Logger &__logger;
		const std::string __appName;
		const std::string __engineName;
		LifeCycle __lifeCycle;

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

		std::unique_ptr<Infra::TemporalDeleter> __pResourceDeleter;
		std::unique_ptr<CommandSubmitter> __pCommandSubmitter;
		std::unique_ptr<InstantCommandExecutor> __pInstantCommandExecutor;
		std::unique_ptr<ScreenManager> __pScreenManager;
		std::unique_ptr<MemoryManager> __pMemoryManager;
		std::unique_ptr<BufferManager> __pBufferManager;

		std::shared_ptr<Infra::EventListener<>> __pCommandEnqueueEventListener;
		std::shared_ptr<Infra::EventListener<>> __pSubmitEventListener;
		std::shared_ptr<Infra::EventListener<size_t>> __pSubmitFinishEventListener;

		static constexpr inline std::string_view VK_KHRONOS_VALIDATION_LAYER_NAME{ "VK_LAYER_KHRONOS_validation" };

		void __initListeners() noexcept;
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
		void __createDescriptorSetLayout();

		void __createResourceDeleter() noexcept;
		void __createCommandSubmitter() noexcept;
		void __createInstantCommandExecutor() noexcept;
		void __createScreenManager() noexcept;
		void __createMemoryManager() noexcept;
		void __createBufferManager() noexcept;

		void __registerListeners() noexcept;

		void __onLifeCycleCommandEnqueue() noexcept;
		void __onLifeCycleCommandSubmit();
		void __onSubmissionFinished(const size_t timestamp) noexcept;

		static VkBool32 VKAPI_PTR vkDebugUtilsMessengerCallbackEXT(
			const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			const VkDebugUtilsMessageTypeFlagsEXT messageTypes,
			const VkDebugUtilsMessengerCallbackDataEXT *const pCallbackData, void *const pUserData);
	};

	constexpr LifeCycle &RenderingEngine::getLifeCycle() noexcept
	{
		return __lifeCycle;
	}
}