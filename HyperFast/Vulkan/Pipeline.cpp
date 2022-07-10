#include "Pipeline.h"
#include <exception>

namespace Vulkan
{
	Pipeline::Pipeline(
		Device &device, const VkPipelineCache pipelineCache,
		const VkGraphicsPipelineCreateInfo &createInfo) :
		Handle{ __create(device, pipelineCache, createInfo) }, __device{ device }
	{}

	Pipeline::~Pipeline() noexcept
	{
		__destroy();
	}

	void Pipeline::__destroy() noexcept
	{
		__device.vkDestroyPipeline(getHandle(), nullptr);
	}

	VkPipeline Pipeline::__create(
		Device &device, const VkPipelineCache pipelineCache,
		const VkGraphicsPipelineCreateInfo &createInfo)
	{
		VkPipeline retVal{};
		device.vkCreateGraphicsPipelines(pipelineCache, 1U, &createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkPipeline." };

		return retVal;
	}
}