#include "RenderingEngine.h"
#include <sstream>
#include <vector>
#include "PhysicalDeviceGroupPicker.h"
#include <shaderc/shaderc.hpp>
#include "../glslc/file_includer.h"

namespace HyperFast
{
	RenderingEngine::RenderingEngine(
		Infra::Logger &logger, const std::string_view &appName, const std::string_view &engineName) :
		__logger{ logger }, __appName { appName }, __engineName{engineName}
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
		__pickPhysicalDeviceGroup();
		__queryPhysicalDeviceProps();
		__retrieveQueueFamilies();
		__createDevice();
		__queryDeviceProc();
		__queryDeviceQueue();
		__createMainCommandPool();
		__createScreenManager();
	}

	RenderingEngine::~RenderingEngine() noexcept
	{
		__pScreenManager = nullptr;
		__destroyMainCommandPool();
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

		/*
			환경 변수 VK_LAYER_PATH를 통해 레이어 경로를 알려주어야 함
			예) VK_LAYER_PATH=$(SolutionDir)ThirdParty\vulkan_layers
		*/
		uint32_t numFoundLayers{};
		globalGlobalProcedure.vkEnumerateInstanceLayerProperties(&numFoundLayers, nullptr);

		std::vector<VkLayerProperties> foundLayers;
		foundLayers.resize(numFoundLayers);
		globalGlobalProcedure.vkEnumerateInstanceLayerProperties(&numFoundLayers, foundLayers.data());

		const void *pNext{};

#ifndef NDEBUG
		const auto foundIt = std::find_if(
			foundLayers.begin(), foundLayers.end(), [](const VkLayerProperties &layer)
		{
			return (VK_KHRONOS_VALIDATION_LAYER_NAME == layer.layerName);
		});

		if (foundIt == foundLayers.end())
			throw std::exception{ "Cannot find the validation layer." };

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

		if (__instance)
			return;

		throw std::exception{ "Cannot create a VkInstance." };
	}

	void RenderingEngine::__destroyInstance() noexcept
	{
		__instanceProc.vkDestroyInstance(__instance, nullptr);
	}

	void RenderingEngine::__queryInstanceProc() noexcept
	{
		__instanceProc = VKL::VulkanLoader::getInstance().queryInstanceProcedure(__instance);
	}

	void RenderingEngine::__createDebugMessenger()
	{
		__instanceProc.vkCreateDebugUtilsMessengerEXT(
			__instance, &__debugMessengerCreateInfo, nullptr, &__debugMessenger);

		if (__debugMessenger)
			return;

		throw std::exception{ "Cannot create a VkDebugUtilsMessengerEXT." };
	}

	void RenderingEngine::__destroyDebugMessenger() noexcept
	{
		__instanceProc.vkDestroyDebugUtilsMessengerEXT(__instance, __debugMessenger, nullptr);
	}

	void RenderingEngine::__pickPhysicalDeviceGroup()
	{
		PhysicalDeviceGroupPicker groupPicker{ __instance, __instanceProc };
		if (!(groupPicker.pick(__physicalDeviceGroupProp)))
			throw std::exception{ "There is a no suitable physical device group." };

		__firstPhysicalDevice = __physicalDeviceGroupProp.physicalDevices[0];
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

		__instanceProc.vkGetPhysicalDeviceProperties2(__firstPhysicalDevice, &__physicalDeviceProp2);
	}

	void RenderingEngine::__retrieveQueueFamilies() noexcept
	{
		uint32_t numProps{};
		__instanceProc.vkGetPhysicalDeviceQueueFamilyProperties(__firstPhysicalDevice, &numProps, nullptr);

		__queueFamilyProps.resize(numProps);
		__instanceProc.vkGetPhysicalDeviceQueueFamilyProperties(__firstPhysicalDevice, &numProps, __queueFamilyProps.data());

		for (uint32_t propIter = 0U; propIter < numProps; propIter++)
		{
			const VkQueueFamilyProperties &queueFamilyProp{ __queueFamilyProps[propIter] };
			if (queueFamilyProp.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
			{
				__graphicsQueueFamilyIndex = propIter;
				break;
			}
		}
	}

	void RenderingEngine::__createDevice()
	{
		const void *pNext{};

		VkPhysicalDeviceVulkan13Features device13Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
		};

		VkPhysicalDeviceVulkan12Features device12Features
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = &device13Features
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

		// 여기서 입력한 배열 순서가 향후 physical device 인덱스로 참조됨
		const VkDeviceGroupDeviceCreateInfo deviceGroupCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_GROUP_DEVICE_CREATE_INFO,
			.pNext = pNext,
			.physicalDeviceCount = __physicalDeviceGroupProp.physicalDeviceCount,
			.pPhysicalDevices = __physicalDeviceGroupProp.physicalDevices
		};

		pNext = &deviceGroupCreateInfo;

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

		__instanceProc.vkCreateDevice(__firstPhysicalDevice, &createInfo, nullptr, &__device);
		if (__device)
			return;

		throw std::exception{ "Cannot create a VkDevice." };
	}

	void RenderingEngine::__destroyDevice() noexcept
	{
		__deviceProc.vkDestroyDevice(__device, nullptr);
	}

	void RenderingEngine::__queryDeviceProc() noexcept
	{
		__deviceProc =
			VKL::VulkanLoader::getInstance().queryDeviceProcedure(__instanceProc.vkGetDeviceProcAddr, __device);
	}

	void RenderingEngine::__queryDeviceQueue()
	{
		__deviceProc.vkGetDeviceQueue(__device, __graphicsQueueFamilyIndex, 0U, &__deviceQueue);
		if (__deviceQueue)
			return;

		throw std::exception{ "Cannot retrieve the device queue." };
	}

	void RenderingEngine::__createMainCommandPool()
	{
		const VkCommandPoolCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = __graphicsQueueFamilyIndex
		};

		__deviceProc.vkCreateCommandPool(__device, &createInfo, nullptr, &__mainCommandPool);
		if (__mainCommandPool)
			return;

		throw std::exception{ "Cannot create the main command pool." };
	}

	void RenderingEngine::__destroyMainCommandPool() noexcept
	{
		__deviceProc.vkDestroyCommandPool(__device, __mainCommandPool, nullptr);
	}

	void RenderingEngine::__createScreenManager() noexcept
	{
		__pScreenManager = std::make_unique<ScreenManager>(
			__instance, __instanceProc, __device, __deviceProc);
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