#include "PhysicalDevicePicker.h"
#include <map>

namespace HyperFast
{
	PhysicalDevicePicker::PhysicalDevicePicker(
		const VkInstance instance, const VKL::InstanceProcedure &instanceProc) noexcept :
		__instance{ instance }, __instanceProc{ instanceProc }
	{}

	VkPhysicalDevice PhysicalDevicePicker::pick() const noexcept
	{
		uint32_t numDevices{};
		__instanceProc.vkEnumeratePhysicalDevices(__instance, &numDevices, nullptr);

		std::vector<VkPhysicalDevice> devices;
		devices.resize(numDevices);
		__instanceProc.vkEnumeratePhysicalDevices(__instance, &numDevices, devices.data());
		
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
		__instanceProc.vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);

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
		__instanceProc.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numProps, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilyProps;
		queueFamilyProps.resize(numProps);
		__instanceProc.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numProps, queueFamilyProps.data());

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
		__instanceProc.vkGetPhysicalDeviceProperties(physicalDevice, &deviceProp);

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