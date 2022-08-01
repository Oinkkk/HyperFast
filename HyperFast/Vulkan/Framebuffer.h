#pragma once

#include "Device.h"

namespace Vulkan
{
	class Framebuffer final : public Handle<VkFramebuffer>
	{
	public:
		Framebuffer(Device &device, const VkFramebufferCreateInfo &createInfo);
		virtual ~Framebuffer() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkFramebuffer __create(
			Device &device, const VkFramebufferCreateInfo &createInfo);
	};
}