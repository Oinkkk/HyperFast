#include "PipelineLayout.h"
#include <exception>

namespace Vulkan
{
	PipelineLayout::PipelineLayout(Device &device, const VkPipelineLayoutCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	PipelineLayout::~PipelineLayout() noexcept
	{
		__destroy();
	}

	void PipelineLayout::__destroy() noexcept
	{
		__device.vkDestroyPipelineLayout(getHandle(), nullptr);
	}

	VkPipelineLayout PipelineLayout::__create(Device &device, const VkPipelineLayoutCreateInfo &createInfo)
	{
		VkPipelineLayout retVal{};
		device.vkCreatePipelineLayout(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkPipelineLayout." };

		return retVal;
	}
}