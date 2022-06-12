#pragma once

#include "VertexAttribute.h"
#include <vulkan/vulkan.h>

namespace HyperFast
{
	class VertexAttributeUtil final
	{
	public:
		[[nodiscard]]
		static constexpr VkFormat getFormatOf(const VertexAttributeFlagBit attributeType) noexcept;

	private:
		VertexAttributeUtil() = delete;
	};

	constexpr VkFormat VertexAttributeUtil::getFormatOf(const VertexAttributeFlagBit attributeType) noexcept
	{
		VkFormat retVal{ VkFormat::VK_FORMAT_UNDEFINED };

		switch (attributeType)
		{
		case VertexAttributeFlagBit::POS:
			retVal = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
			break;

		case VertexAttributeFlagBit::COLOR:
			retVal = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
			break;
		}

		return retVal;
	}
}
