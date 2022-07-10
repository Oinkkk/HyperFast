#pragma once

#include "Device.h"

namespace Vulkan
{
	class RenderPass final : public Handle<VkRenderPass>
	{
	public:
		RenderPass(Device &device, const VkRenderPassCreateInfo2 &createInfo);
		~RenderPass() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkRenderPass __create(
			Device &device, const VkRenderPassCreateInfo2 &createInfo);
	};
}