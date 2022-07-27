#include "DescriptorSetLayout.h"
#include <exception>

namespace Vulkan
{
	DescriptorSetLayout::DescriptorSetLayout(
		Device &device, const VkDescriptorSetLayoutCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	DescriptorSetLayout::~DescriptorSetLayout() noexcept
	{
		__destroy();
	}

	void DescriptorSetLayout::__destroy() noexcept
	{
		__device.vkDestroyDescriptorSetLayout(getHandle(), nullptr);
	}

	VkDescriptorSetLayout DescriptorSetLayout::__create(
		Device &device, const VkDescriptorSetLayoutCreateInfo &createInfo)
	{
		VkDescriptorSetLayout retVal{};
		device.vkCreateDescriptorSetLayout(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkDescriptorSetLayout." };

		return retVal;
	}
}