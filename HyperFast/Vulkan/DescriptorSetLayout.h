#pragma once

#include "Device.h"

namespace Vulkan
{
	class DescriptorSetLayout final : public Handle<VkDescriptorSetLayout>
	{
	public:
		DescriptorSetLayout(Device &device, const VkDescriptorSetLayoutCreateInfo &createInfo);
		~DescriptorSetLayout() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkDescriptorSetLayout __create(
			Device &device, const VkDescriptorSetLayoutCreateInfo &createInfo);
	};
}