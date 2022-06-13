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
		__getInstanceVersion();
		__checkInstanceVersionSupport();

		#ifndef NDEBUG
		__populateDebugMessengerCreateInfo();
		#endif

		__createInstance();
		__queryInstanceProc();

		#ifndef NDEBUG
		__createDebugMessenger();
		#endif

		__pickPhysicalDevice();
		__queryPhysicalDeviceProps();
		__retrieveQueueFamilies();
		__createDevice();
		__queryDeviceProc();
		__queryGraphicsQueue();
		__createScreenManager();
	}

	RenderingEngine::~RenderingEngine() noexcept
	{
		__waitDeviceIdle();
		__destroyScreenManager();
		__destroyDevice();

		#ifndef NDEBUG
		__destroyDebugMessenger();
		#endif

		__destroyInstance();
	}

	ScreenManager &RenderingEngine::getScreenManager() noexcept
	{
		return *__pScreenManager;
	}

	void RenderingEngine::__getInstanceVersion() noexcept
	{
		const VKL::GlobalProcedure &globalGlobalProcedure{ VKL::VulkanLoader::getInstance().getGlobalProcedure() };
		if (!(globalGlobalProcedure.vkEnumerateInstanceVersion))
		{
			__instanceVersion = VK_API_VERSION_1_0;
			return;
		}

		globalGlobalProcedure.vkEnumerateInstanceVersion(&__instanceVersion);
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
		const VKL::GlobalProcedure &globalGlobalProcedure{ VKL::VulkanLoader::getInstance().getGlobalProcedure() };

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

		globalGlobalProcedure.vkCreateInstance(&createInfo, nullptr, &__instance);
		if (!__instance)
			throw std::exception{ "Cannot create a VkInstance." };
	}

	void RenderingEngine::__destroyInstance() noexcept
	{
		__instanceProc.vkDestroyInstance(__instance, nullptr);
		__instance = VK_NULL_HANDLE;
	}

	void RenderingEngine::__queryInstanceProc() noexcept
	{
		__instanceProc = VKL::VulkanLoader::getInstance().queryInstanceProcedure(__instance);
	}

	void RenderingEngine::__createDebugMessenger()
	{
		__instanceProc.vkCreateDebugUtilsMessengerEXT(
			__instance, &__debugMessengerCreateInfo, nullptr, &__debugMessenger);

		if (!__debugMessenger)
			throw std::exception{ "Cannot create a VkDebugUtilsMessengerEXT." };
	}

	void RenderingEngine::__destroyDebugMessenger() noexcept
	{
		__instanceProc.vkDestroyDebugUtilsMessengerEXT(__instance, __debugMessenger, nullptr);
		__debugMessenger = VK_NULL_HANDLE;
	}

	void RenderingEngine::__pickPhysicalDevice()
	{
		PhysicalDevicePicker groupPicker{ __instance, __instanceProc };
		
		__physicalDevice = groupPicker.pick();
		if (!__physicalDevice)
			throw std::exception{ "There is a no suitable physical device group." };
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

		__instanceProc.vkGetPhysicalDeviceProperties2(__physicalDevice, &__physicalDeviceProp2);
	}

	void RenderingEngine::__retrieveQueueFamilies() noexcept
	{
		uint32_t numProps{};
		__instanceProc.vkGetPhysicalDeviceQueueFamilyProperties(__physicalDevice, &numProps, nullptr);

		__queueFamilyProps.resize(numProps);
		__instanceProc.vkGetPhysicalDeviceQueueFamilyProperties(__physicalDevice, &numProps, __queueFamilyProps.data());

		for (uint32_t propIter = 0U; propIter < numProps; propIter++)
		{
			const VkQueueFamilyProperties &queueFamilyProp{ __queueFamilyProps[propIter] };

			if (!(queueFamilyProp.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT))
				continue;

			const VkBool32 win32SupportResult
			{
				__instanceProc.vkGetPhysicalDeviceWin32PresentationSupportKHR(__physicalDevice, propIter)
			};

			if (!win32SupportResult)
				continue;

			__graphicsQueueFamilyIndex = propIter;
			break;
		}
	}

	void RenderingEngine::__createDevice()
	{
		const void *pNext{};

		VkPhysicalDeviceVulkan13Features device13Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
			.pipelineCreationCacheControl = VK_TRUE,
			.synchronization2 = VK_TRUE
		};

		VkPhysicalDeviceVulkan12Features device12Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &device13Features,
			.imagelessFramebuffer = VK_TRUE
		};

		VkPhysicalDeviceVulkan11Features device11Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
			.pNext = &device12Features
		};

		const VkPhysicalDeviceFeatures2 deviceFeatures2
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext = &device11Features,
		};

		pNext = &deviceFeatures2;

		static constexpr float queuePriority{ 1.f };
		const VkDeviceQueueCreateInfo queueCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
			.queueFamilyIndex = __graphicsQueueFamilyIndex,
			.queueCount = 1U,
			.pQueuePriorities = &queuePriority
		};

		std::vector<const char *> enabledExtensions;
		enabledExtensions.emplace_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

		const VkDeviceCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext = pNext,
			.queueCreateInfoCount = 1U,
			.pQueueCreateInfos = &queueCreateInfo,
			.enabledExtensionCount = uint32_t(enabledExtensions.size()),
			.ppEnabledExtensionNames = enabledExtensions.data()
		};

		__instanceProc.vkCreateDevice(__physicalDevice, &createInfo, nullptr, &__device);
		if (!__device)
			throw std::exception{ "Cannot create a VkDevice." };
	}

	void RenderingEngine::__destroyDevice() noexcept
	{
		__deviceProc.vkDestroyDevice(__device, nullptr);
		__device = VK_NULL_HANDLE;
	}

	void RenderingEngine::__queryDeviceProc() noexcept
	{
		__deviceProc =
			VKL::VulkanLoader::getInstance().queryDeviceProcedure(__instanceProc.vkGetDeviceProcAddr, __device);
	}

	void RenderingEngine::__queryGraphicsQueue()
	{
		__deviceProc.vkGetDeviceQueue(__device, __graphicsQueueFamilyIndex, 0U, &__graphicsQueue);
		if (!__graphicsQueue)
			throw std::exception{ "Cannot retrieve the graphics queue." };
	}

	void RenderingEngine::__createScreenManager() noexcept
	{
		__pScreenManager = std::make_unique<ScreenManager>(
			__instance, __instanceProc,
			__physicalDevice, __graphicsQueueFamilyIndex,
			__device, __deviceProc, __graphicsQueue, __logger);
	}

	void RenderingEngine::__destroyScreenManager() noexcept
	{
		__pScreenManager = nullptr;
	}

	void RenderingEngine::__waitDeviceIdle() const noexcept
	{
		__deviceProc.vkDeviceWaitIdle(__device);
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