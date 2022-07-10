#pragma once

#include "Device.h"

namespace Vulkan
{
	class PipelineCache final : public Handle<VkPipelineCache>
	{
	public:
		PipelineCache(Device &device, const VkPipelineCacheCreateInfo &createInfo);
		~PipelineCache() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkPipelineCache __create(
			Device &device, const VkPipelineCacheCreateInfo &createInfo);
	};
}