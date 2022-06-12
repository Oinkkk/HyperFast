#pragma once

#include <vulkan/vulkan.h>

namespace HyperFast
{
	class SizeUtil final
	{
	public:
		[[nodiscard]]
		static constexpr size_t getElementSizeOf(const VkFormat format) noexcept;

	private:
		SizeUtil() = delete;
	};

	constexpr size_t SizeUtil::getElementSizeOf(const VkFormat format) noexcept
	{
		size_t retVal{};

		switch (format)
		{
		case VkFormat::VK_FORMAT_R32G32_SFLOAT:
			retVal = 8ULL;
			break;

		case VkFormat::VK_FORMAT_R32G32B32_SFLOAT:
			retVal = 12ULL;
			break;

		case VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT:
			retVal = 16ULL;
			break;
		}

		return retVal;
	}
}