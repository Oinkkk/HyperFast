#pragma once

#include "Device.h"

namespace Vulkan
{
	class PipelineLayout final : public Handle<VkPipelineLayout>
	{
	public:
		PipelineLayout(Device &device, const VkPipelineLayoutCreateInfo &createInfo);
		virtual ~PipelineLayout() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkPipelineLayout __create(
			Device &device, const VkPipelineLayoutCreateInfo &createInfo);
	};
}