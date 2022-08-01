#pragma once

#include "Device.h"

namespace Vulkan
{
	class ImageView final : public Handle<VkImageView>
	{
	public:
		ImageView(Device &device, const VkImageViewCreateInfo &createInfo);
		virtual ~ImageView() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkImageView __create(
			Device &device, const VkImageViewCreateInfo &createInfo);
	};
}