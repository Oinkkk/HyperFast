#include "PhysicalDevicePicker.h"
#include <map>
#include <vector>

namespace HyperFast
{
	PhysicalDevicePicker::PhysicalDevicePicker(Vulkan::Instance &instance) noexcept :
		__instance{ instance }
	{}

	VkPhysicalDevice PhysicalDevicePicker::pick() const noexcept
	{
		uint32_t numDevices{};
		__instance.vkEnumeratePhysicalDevices(&numDevices, nullptr);

		std::vector<VkPhysicalDevice> devices;
		devices.resize(numDevices);
		__instance.vkEnumeratePhysicalDevices(&numDevices, devices.data());
		
		std::multimap<uint32_t, VkPhysicalDevice, std::greater<uint32_t>> scoreMap;
		for (const VkPhysicalDevice device : devices)
		{
			if (!__checkVersionSupport(device))
				continue;

			if (!__checkQueueSupport(device))
				continue;

			const uint32_t score{ __getScoreOf(device) };
			scoreMap.emplace(score, device);
		}

		if (scoreMap.empty())
			return VK_NULL_HANDLE;

		return scoreMap.begin()->second;
	}

	bool PhysicalDevicePicker::__checkVersionSupport(const VkPhysicalDevice physicalDevice) const noexcept
	{
		VkPhysicalDeviceProperties physicalDeviceProperties{};
		__instance.vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

		const uint32_t deviceVersion{ physicalDeviceProperties.apiVersion };

		const uint32_t major{ VK_API_VERSION_MAJOR(deviceVersion) };
		const uint32_t minor{ VK_API_VERSION_MINOR(deviceVersion) };
		const uint32_t patch{ VK_API_VERSION_PATCH(deviceVersion) };
		const uint32_t variant{ VK_API_VERSION_VARIANT(deviceVersion) };

		if (major > 1U)
			return true;

		return ((major == 1U) && (minor >= 3U));
	}

	bool PhysicalDevicePicker::__checkQueueSupport(const VkPhysicalDevice physicalDevice) const noexcept
	{
		uint32_t numProps{};
		__instance.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numProps, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilyProps;
		queueFamilyProps.resize(numProps);
		__instance.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numProps, queueFamilyProps.data());

		for (uint32_t propIter = 0U; propIter < numProps; propIter++)
		{
			const VkQueueFamilyProperties &queueFamilyProp{ queueFamilyProps[propIter] };

			if (queueFamilyProp.queueFlags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
				return true;
		}

		return false;
	}

	uint32_t PhysicalDevicePicker::__getScoreOf(const VkPhysicalDevice physicalDevice) const noexcept
	{
		VkPhysicalDeviceProperties deviceProp{};
		__instance.vkGetPhysicalDeviceProperties(physicalDevice, &deviceProp);

		uint32_t retVal{};

		switch (deviceProp.deviceType)
		{
		case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
			retVal += 10000U;
			break;

		case VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
			retVal += 1000U;
			break;
		}

		return retVal;
	}
}