#include "PhysicalDeviceGroupPicker.h"
#include <map>

namespace HyperFast
{
	PhysicalDeviceGroupPicker::PhysicalDeviceGroupPicker(
		const VkInstance instance, VKL::InstanceProcedure &instanceProc) noexcept :
		__instance{ instance }, __instanceProc{ instanceProc }
	{}

	bool PhysicalDeviceGroupPicker::pick(VkPhysicalDeviceGroupProperties &physicalDeviceGroupProp) const noexcept
	{
		uint32_t numGroups{};
		__instanceProc.vkEnumeratePhysicalDeviceGroups(__instance, &numGroups, nullptr);

		std::vector<VkPhysicalDeviceGroupProperties> groupProps;
		groupProps.resize(numGroups);
		__instanceProc.vkEnumeratePhysicalDeviceGroups(__instance, &numGroups, groupProps.data());
		
		std::multimap<uint32_t, uint32_t, std::greater<uint32_t>> scoreMap;

		for (uint32_t groupIter = 0U; groupIter < numGroups; groupIter++)
		{
			const VkPhysicalDeviceGroupProperties &groupProp{ groupProps[groupIter] };
			const VkPhysicalDevice firstDevice{ groupProp.physicalDevices[0] };
			
			if (!__checkVersionSupport(firstDevice))
				continue;

			if (!__checkQueueSupport(firstDevice))
				continue;

			const uint32_t score{ __getScoreOf(groupProp) };
			scoreMap.emplace(score, groupIter);
		}

		if (scoreMap.empty())
			return false;

		const uint32_t groupIdx{ scoreMap.begin()->second };
		physicalDeviceGroupProp = groupProps[groupIdx];

		return true;
	}

	bool PhysicalDeviceGroupPicker::__checkVersionSupport(const VkPhysicalDevice physicalDevice) const noexcept
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

	bool PhysicalDeviceGroupPicker::__checkQueueSupport(const VkPhysicalDevice physicalDevice) const noexcept
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

	uint32_t PhysicalDeviceGroupPicker::__getScoreOf(
		const VkPhysicalDeviceGroupProperties &physicalDeviceGroupProp) const noexcept
	{
		const VkPhysicalDevice firstDevice{ physicalDeviceGroupProp.physicalDevices[0] };

		VkPhysicalDeviceProperties deviceProp{};
		__instanceProc.vkGetPhysicalDeviceProperties(firstDevice, &deviceProp);

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