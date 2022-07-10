#include "Pipeline.h"
#include <exception>

namespace Vulkan
{
	Pipeline::Pipeline(
		Device &device, const VkPipelineCache pipelineCache,
		const VkGraphicsPipelineCreateInfo &createInfos) :
		Handle{ __create(device, pipelineCache, createInfos) }, __device{ device }
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
		const VkGraphicsPipelineCreateInfo &createInfos)
	{
		VkPipeline retVal{};
		device.vkCreateGraphicsPipelines(pipelineCache, 1U, &createInfos, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkPipeline." };

		return retVal;
	}
}