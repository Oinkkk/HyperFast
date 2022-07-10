#include "RenderPass.h"
#include <exception>

namespace Vulkan
{
	RenderPass::RenderPass(Device &device, const VkRenderPassCreateInfo2 &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	RenderPass::~RenderPass() noexcept
	{
		__destroy();
	}

	void RenderPass::__destroy() noexcept
	{
		__device.vkDestroyRenderPass(getHandle(), nullptr);
	}

	VkRenderPass RenderPass::__create(Device &device, const VkRenderPassCreateInfo2 &createInfo)
	{
		VkRenderPass retVal{};
		device.vkCreateRenderPass2(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkRenderPass." };

		return retVal;
	}
}