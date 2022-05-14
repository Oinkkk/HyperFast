#include "RenderingEngine.h"
#include <sstream>
#include <vector>

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

		__retrievePhysicalDevice();
		__queryPhysicalDeviceProperties();
	}

	RenderingEngine::~RenderingEngine() noexcept
	{
		__resetPhysicalDeviceProperties();
		__resetPhysicalDevice();

#ifndef NDEBUG
		__destroyDebugMessenger();
#endif

		__destroyInstance();
		__resetInstanceProc();
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
			VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT		|
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
			VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_EXT,
			VkValidationFeatureEnableEXT::VK_VALIDATION_FEATURE_ENABLE_GPU_ASSISTED_RESERVE_BINDING_SLOT_EXT,
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
		__instance = nullptr;
	}

	void RenderingEngine::__queryInstanceProc() noexcept
	{
		__instanceProc = VKL::VulkanLoader::getInstance().queryInstanceProcedure(__instance);
	}

	void RenderingEngine::__resetInstanceProc() noexcept
	{
		__instanceProc = {};
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
		__debugMessenger = nullptr;
	}

	bool RenderingEngine::__checkDeviceVersionSupport(const VkPhysicalDevice device) const noexcept
	{
		VkPhysicalDeviceProperties physicalDeviceProperties{};
		__instanceProc.vkGetPhysicalDeviceProperties(device, &physicalDeviceProperties);

		const uint32_t deviceVersion{ physicalDeviceProperties.apiVersion };

		const uint32_t major{ VK_API_VERSION_MAJOR(deviceVersion) };
		const uint32_t minor{ VK_API_VERSION_MINOR(deviceVersion) };
		const uint32_t patch{ VK_API_VERSION_PATCH(deviceVersion) };
		const uint32_t variant{ VK_API_VERSION_VARIANT(deviceVersion) };

		if (major > 1U)
			return true;

		return ((major == 1U) && (minor >= 3U));
	}

	void RenderingEngine::__retrievePhysicalDevice()
	{
		uint32_t numDevices{};
		__instanceProc.vkEnumeratePhysicalDevices(__instance, &numDevices, nullptr);

		if (!numDevices)
			throw std::exception{ "There are no physical devices." };

		std::vector<VkPhysicalDevice> devices;
		devices.resize(numDevices);
		__instanceProc.vkEnumeratePhysicalDevices(__instance, &numDevices, devices.data());

		for (const VkPhysicalDevice device : devices)
		{
			if (!__checkDeviceVersionSupport(device))
				continue;

			uint32_t numProps{};
			__instanceProc.vkGetPhysicalDeviceQueueFamilyProperties(device, &numProps, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilyProps;
			queueFamilyProps.resize(numProps);
			__instanceProc.vkGetPhysicalDeviceQueueFamilyProperties(device, &numProps, queueFamilyProps.data());

			bool graphicsFound{};
			bool dmaTransferFound{};
			for (uint32_t propIter = 0U; propIter < numProps; propIter++)
			{
				const VkQueueFamilyProperties &queueFamilyProp{ queueFamilyProps[propIter] };
				if (!graphicsFound && (queueFamilyProp.queueFlags == VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT))
				{
					dmaTransferFound = true;
					__transferQueueFamilyIndex = propIter;
				}

				if (!graphicsFound && (queueFamilyProp.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT))
				{
					graphicsFound = true;
					__graphicsQueueFamilyIndex = propIter;
				}
			}

			if (graphicsFound)
			{
				__queueFamilyProps.swap(queueFamilyProps);
				if (!dmaTransferFound)
					__transferQueueFamilyIndex = __graphicsQueueFamilyIndex;
			}

			if (__queueFamilyProps.empty())
				continue;

			__physicalDevice = device;
			break;
		}

		if (!__physicalDevice)
			throw std::exception{ "There are no suitable physical devices." };
	}

	void RenderingEngine::__resetPhysicalDevice() noexcept
	{
		__queueFamilyProps.clear();
		__graphicsQueueFamilyIndex = 0U;
		__transferQueueFamilyIndex = 0U;

		__physicalDevice = nullptr;
	}

	void RenderingEngine::__queryPhysicalDeviceProperties() noexcept
	{
		__physicalDeviceProperties2.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
		__instanceProc.vkGetPhysicalDeviceProperties2(__physicalDevice, &__physicalDeviceProperties2);
	}

	void RenderingEngine::__resetPhysicalDeviceProperties() noexcept
	{
		__physicalDeviceProperties2 = {};
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