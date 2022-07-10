#include "ImageView.h"
#include <exception>

namespace Vulkan
{
	ImageView::ImageView(Device &device, const VkImageViewCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	ImageView::~ImageView() noexcept
	{
		__destroy();
	}

	void ImageView::__destroy() noexcept
	{
		__device.vkDestroyImageView(getHandle(), nullptr);
	}

	VkImageView ImageView::__create(Device &device, const VkImageViewCreateInfo &createInfo)
	{
		VkImageView retVal{};
		device.vkCreateImageView(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkImageView." };

		return retVal;
	}
}