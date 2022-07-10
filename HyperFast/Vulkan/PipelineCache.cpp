#include "PipelineCache.h"
#include <exception>

namespace Vulkan
{
	PipelineCache::PipelineCache(Device &device, const VkPipelineCacheCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	PipelineCache::~PipelineCache() noexcept
	{
		__destroy();
	}

	void PipelineCache::__destroy() noexcept
	{
		__device.vkDestroyPipelineCache(getHandle(), nullptr);
	}

	VkPipelineCache PipelineCache::__create(Device &device, const VkPipelineCacheCreateInfo &createInfo)
	{
		VkPipelineCache retVal{};
		device.vkCreatePipelineCache(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkPipelineCache." };

		return retVal;
	}
}