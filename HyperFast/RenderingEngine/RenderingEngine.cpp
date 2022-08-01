#include "RenderingEngine.h"
#include <sstream>
#include <vector>
#include "PhysicalDevicePicker.h"
#include <shaderc/shaderc.hpp>
#include "../glslc/file_includer.h"

namespace HyperFast
{
	RenderingEngine::RenderingEngine(
		Infra::Logger &logger, const std::string_view &appName, const std::string_view &engineName) :
		__logger{ logger }, __appName{ appName }, __engineName{ engineName }
	{
		__initListeners();
		__getInstanceVersion();
		__checkInstanceVersionSupport();

#ifndef NDEBUG
		__populateDebugMessengerCreateInfo();
#endif

		__createInstance();

#ifndef NDEBUG
		__createDebugMessenger();
#endif

		__pickPhysicalDevice();
		__queryPhysicalDeviceProps();
		__pickGraphicsQueueFamily();
		__createDevice();
		__makeQueue();

		__createResourceDeleter();
		__createCommandSubmitter();
		__createScreenManager();
		__createMemoryManager();
		__createBufferManager();

		__registerListeners();
	}

	RenderingEngine::~RenderingEngine() noexcept
	{
		// The only time you should wait for a device to idle is when you want to destroy the device.
		__pDevice->vkDeviceWaitIdle();
		__pResourceDeleter = nullptr;
		__pCommandSubmitter = nullptr;
		__pBufferManager = nullptr;
		__pMemoryManager = nullptr;
		__pScreenManager = nullptr;
		__pDevice = nullptr;
		__pDebugMessenger = nullptr;
		__pInstance = nullptr;
	}

	std::shared_ptr<Screen> RenderingEngine::createScreen(Win::Window &window)
	{
		return std::make_shared<HyperFast::Screen>(*__pScreenManager, window);
	}

	std::unique_ptr<Buffer> RenderingEngine::createBuffer(
		const VkDeviceSize size, const VkBufferUsageFlags usage)
	{
		return std::make_unique<Buffer>(*__pBufferManager, size, usage);
	}

	std::shared_ptr<Memory> RenderingEngine::createMemory(
		const VkMemoryRequirements &memRequirements,
		const VkMemoryPropertyFlags requiredProps, const bool linearity)
	{
		return std::make_shared<Memory>(
			*__pMemoryManager, memRequirements, requiredProps, linearity);
	}

	std::unique_ptr<Drawcall> RenderingEngine::createDrawcall() noexcept
	{
		return std::make_unique<Drawcall>(
			*__pDevice, __queueFamilyIndex, *__pBufferManager, *__pMemoryManager);
	}

	std::shared_ptr<Mesh> RenderingEngine::createMesh() noexcept
	{
		return std::make_shared<Mesh>(*__pDevice);
	}

	void RenderingEngine::tick()
	{
		__lifeCycle.tick();
	}

	void RenderingEngine::__initListeners() noexcept
	{
		__pSubmitEventListener = Infra::EventListener<>::make([this]
		{
			const bool validSubmission{ __pCommandSubmitter->submit() };
			if (validSubmission)
				__pResourceDeleter->advance();
		});

		__pSubmitFinishEventListener = Infra::EventListener<size_t>::make([this](const size_t timestamp)
		{
			__pResourceDeleter->commit(timestamp);
		});
	}

	void RenderingEngine::__getInstanceVersion() noexcept
	{
		const Vulkan::GlobalProcedure &globalProcedure
		{ 
			Vulkan::VulkanLoader::getInstance().getGlobalProcedure()
		};

		if (!(globalProcedure.vkEnumerateInstanceVersion))
		{
			__instanceVersion = VK_API_VERSION_1_0;
			return;
		}

		globalProcedure.vkEnumerateInstanceVersion(&__instanceVersion);
	}

	void RenderingEngine::__checkInstanceVersionSupport() const
	{
		const uint32_t major{ VK_API_VERSION_MAJOR(__instanceVersion) };
		const uint32_t minor{ VK_API_VERSION_MINOR(__instanceVersion) };
		const uint32_t patch{ VK_API_VERSION_PATCH(__instanceVersion) };
		const uint32_t variant{ VK_API_VERSION_VARIANT(__instanceVersion) };

		if (major > 1U)
			return;

		if ((major == 1U) && (minor >= 3U))
			return;

		std::ostringstream oss;
		oss << "The instance version is too low. ";
		oss << "Expected: >= 1.3, Actual: ";
		oss << major << '.' << minor << '.' << patch << '.' << variant;

		throw std::exception{ oss.str().c_str() };
	}

	void RenderingEngine::__populateDebugMessengerCreateInfo() noexcept
	{
		__debugMessengerCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		__debugMessengerCreateInfo.messageSeverity =
		(
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT		|
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT		|
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT
		);

		__debugMessengerCreateInfo.messageType =
		(
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT		|
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT	|
			VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT
		);

		__debugMessengerCreateInfo.pfnUserCallback = vkDebugUtilsMessengerCallbackEXT;
		__debugMessengerCreateInfo.pUserData = &__logger;
	}

	void RenderingEngine::__createInstance()
	{
		const Vulkan::GlobalProcedure &globalProc{ Vulkan::VulkanLoader::getInstance().getGlobalProcedure() };

		std::vector<const char *> enabledLayers;
		std::vector<const char *> enabledExtensions;
		const void *pNext{};

#ifndef NDEBUG
		/*
			환경 변수 VK_LAYER_PATH를 통해 레이어 경로를 알려주어야 함
			예) VK_LAYER_PATH=$(SolutionDir)ThirdParty\vulkan_layers
		*/
		enabledLayers.emplace_back(VK_KHRONOS_VALIDATION_LAYER_NAME.data());
		enabledExtensions.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		pNext = &__debugMessengerCreateInfo;

		static constexpr VkValidationFeatureEnableEXT enabledValidationFeatures[]
		{
			VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_BEST_PRACTICES_EXT,
			VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT,
			VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_SYNCHRONIZATION_VALIDATION_EXT
		};

		static constexpr VkValidationFeatureDisableEXT disabledValidationFeatures[]
		{
			VkValidationFeatureDisableEXT::VK_VALIDATION_FEATURE_DISABLE_SHADER_VALIDATION_CACHE_EXT
		};

		const VkValidationFeaturesEXT validationFeatures
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT,
			.pNext = pNext,
			.enabledValidationFeatureCount = uint32_t(std::size(enabledValidationFeatures)),
			.pEnabledValidationFeatures = enabledValidationFeatures,
			.disabledValidationFeatureCount = uint32_t(std::size(disabledValidationFeatures)),
			.pDisabledValidationFeatures = disabledValidationFeatures
		};

		pNext = &validationFeatures;
#endif

		enabledExtensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);
		enabledExtensions.emplace_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

		const VkApplicationInfo appInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName = __appName.c_str(),
			.applicationVersion = VK_API_VERSION_1_0,
			.pEngineName = __engineName.c_str(),
			.engineVersion = VK_API_VERSION_1_0,
			.apiVersion = __instanceVersion
		};

		const VkInstanceCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext = pNext,
			.pApplicationInfo = &appInfo,
			.enabledLayerCount = uint32_t(enabledLayers.size()),
			.ppEnabledLayerNames = enabledLayers.data(),
			.enabledExtensionCount = uint32_t(enabledExtensions.size()),
			.ppEnabledExtensionNames = enabledExtensions.data()
		};

		__pInstance = std::make_unique<Vulkan::Instance>(createInfo);
	}

	void RenderingEngine::__createDebugMessenger()
	{
		__pDebugMessenger =
			std::make_unique<Vulkan::DebugUtilsMessenger>(*__pInstance, __debugMessengerCreateInfo);
	}

	void RenderingEngine::__pickPhysicalDevice()
	{
		PhysicalDevicePicker groupPicker{ *__pInstance };
		__pPhysicalDevice = std::make_unique<Vulkan::PhysicalDevice>(*__pInstance, groupPicker.pick());
	}

	void RenderingEngine::__queryPhysicalDeviceProps() noexcept
	{
		__physicalDevice13Prop.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_PROPERTIES;
		__physicalDevice12Prop.pNext = nullptr;

		__physicalDevice12Prop.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_PROPERTIES;
		__physicalDevice12Prop.pNext = &__physicalDevice13Prop;

		__physicalDevice11Prop.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_PROPERTIES;
		__physicalDevice11Prop.pNext = &__physicalDevice12Prop;

		__physicalDeviceProp2.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		__physicalDeviceProp2.pNext = &__physicalDevice11Prop;

		__pPhysicalDevice->vkGetPhysicalDeviceProperties2(&__physicalDeviceProp2);
	}

	void RenderingEngine::__pickGraphicsQueueFamily() noexcept
	{
		uint32_t numProps{};
		__pPhysicalDevice->vkGetPhysicalDeviceQueueFamilyProperties(&numProps, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilyProps;
		queueFamilyProps.resize(numProps);

		__pPhysicalDevice->vkGetPhysicalDeviceQueueFamilyProperties(&numProps, queueFamilyProps.data());

		for (uint32_t propIter = 0U; propIter < numProps; propIter++)
		{
			const VkQueueFamilyProperties &queueFamilyProp{ queueFamilyProps[propIter] };

			if (!(queueFamilyProp.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT))
				continue;

			const VkBool32 win32SupportResult
			{
				__pPhysicalDevice->vkGetPhysicalDeviceWin32PresentationSupportKHR(propIter)
			};

			if (!win32SupportResult)
				continue;

			__queueFamilyIndex = propIter;
			break;
		}
	}

	void RenderingEngine::__createDevice()
	{
		const void *pNext{};

		VkPhysicalDeviceRobustness2FeaturesEXT deviceRobustness2Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ROBUSTNESS_2_FEATURES_EXT,
			.nullDescriptor = VK_TRUE
		};

		VkPhysicalDeviceVulkan13Features device13Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pNext = &deviceRobustness2Features,
			.pipelineCreationCacheControl = VK_TRUE,
			.synchronization2 = VK_TRUE
		};

		VkPhysicalDeviceVulkan12Features device12Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &device13Features,
			.drawIndirectCount = VK_TRUE,
			.imagelessFramebuffer = VK_TRUE,
			.timelineSemaphore = VK_TRUE
		};

		VkPhysicalDeviceVulkan11Features device11Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.pNext = &device12Features
		};

		VkPhysicalDeviceFeatures device10Features
		{
			.multiDrawIndirect = VK_TRUE
		};

		VkPhysicalDeviceFeatures2 deviceFeatures2
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &device11Features,
			.features = device10Features
		};

		pNext = &deviceFeatures2;

		static constexpr float queuePriority{ 1.f };
		const VkDeviceQueueCreateInfo queueCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = __queueFamilyIndex,
			.queueCount = 1U,
			.pQueuePriorities = &queuePriority
		};

		std::vector<const char *> enabledExtensions;
		enabledExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		enabledExtensions.emplace_back(VK_EXT_ROBUSTNESS_2_EXTENSION_NAME);

		const VkDeviceCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = pNext,
			.queueCreateInfoCount = 1U,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = uint32_t(enabledExtensions.size()),
			.ppEnabledExtensionNames = enabledExtensions.data()
		};

		__pDevice = std::make_unique<Vulkan::Device>(*__pInstance, *__pPhysicalDevice, createInfo);
	}

	void RenderingEngine::__makeQueue() noexcept
	{
		__pQueue = std::make_unique<Vulkan::Queue>(*__pDevice, __queueFamilyIndex, 0U);
	}

	void RenderingEngine::__createDescriptorSetLayout()
	{
		/*const VkDescriptorSetLayoutBinding binding
		{
			.binding = DESCRIPTOR_SET_BINDING_TRANSFORM,
			.descriptorType = VkDescriptorType::VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount;
			.stageFlags;
			.pImmutableSamplers;
		};*/
	}

	void RenderingEngine::__createResourceDeleter() noexcept
	{
		__pResourceDeleter = std::make_unique<Infra::TemporalDeleter>();
	}

	void RenderingEngine::__createCommandSubmitter() noexcept
	{
		__pCommandSubmitter = std::make_unique<CommandSubmitter>(*__pDevice, *__pQueue);
	}

	void RenderingEngine::__createScreenManager() noexcept
	{
		__pScreenManager = std::make_unique<ScreenManager>(
			*__pInstance, *__pPhysicalDevice, __queueFamilyIndex,
			*__pDevice, *__pQueue, __lifeCycle, *__pCommandSubmitter, *__pResourceDeleter);
	}

	void RenderingEngine::__createMemoryManager() noexcept
	{
		__pMemoryManager = std::make_unique<MemoryManager>(
			*__pInstance, *__pPhysicalDevice, *__pDevice, *__pResourceDeleter);
	}

	void RenderingEngine::__createBufferManager() noexcept
	{
		__pBufferManager = std::make_unique<BufferManager>(*__pDevice, *__pResourceDeleter);
	}

	void RenderingEngine::__registerListeners() noexcept
	{
		__lifeCycle.getSignalEvent(LifeCycleType::SUBMIT) += __pSubmitEventListener;
		__pCommandSubmitter->getFinishEvent() += __pSubmitFinishEventListener;
	}

	VkBool32 VKAPI_PTR RenderingEngine::vkDebugUtilsMessengerCallbackEXT(
		const VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		const VkDebugUtilsMessageTypeFlagsEXT messageTypes,
		const VkDebugUtilsMessengerCallbackDataEXT *const pCallbackData, void *const pUserData)
	{
		Infra::Logger *const pLogger{ reinterpret_cast<Infra::Logger *>(pUserData) };

		Infra::LogSeverityType severityType{};
		switch (messageSeverity)
		{
		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
			severityType = Infra::LogSeverityType::VERBOSE;
			break;

		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
			severityType = Infra::LogSeverityType::INFO;
			break;

		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			severityType = Infra::LogSeverityType::WARNING;
			break;

		case VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			severityType = Infra::LogSeverityType::FATAL;
			break;
		}

		pLogger->log(severityType, pCallbackData->pMessage);
		return VK_FALSE;
	}
}