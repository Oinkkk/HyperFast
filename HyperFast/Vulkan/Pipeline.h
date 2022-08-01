#pragma once

#include "Device.h"

namespace Vulkan
{
	class Pipeline final : public Handle<VkPipeline>
	{
	public:
		Pipeline(
			Device &device, const VkPipelineCache pipelineCache,
			const VkGraphicsPipelineCreateInfo &createInfo);

		virtual ~Pipeline() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkPipeline __create(
			Device &device, const VkPipelineCache pipelineCache,
			const VkGraphicsPipelineCreateInfo &createInfo);
	};
}