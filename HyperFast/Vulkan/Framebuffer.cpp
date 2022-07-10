#include "Framebuffer.h"
#include <exception>

namespace Vulkan
{
	Framebuffer::Framebuffer(Device &device, const VkFramebufferCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	Framebuffer::~Framebuffer() noexcept
	{
		__destroy();
	}

	void Framebuffer::__destroy() noexcept
	{
		__device.vkDestroyFramebuffer(getHandle(), nullptr);
	}

	VkFramebuffer Framebuffer::__create(Device &device, const VkFramebufferCreateInfo &createInfo)
	{
		VkFramebuffer retVal{};
		device.vkCreateFramebuffer(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkImageView." };

		return retVal;
	}
}