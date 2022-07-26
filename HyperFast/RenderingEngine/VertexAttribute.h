#pragma once

#include "Constant.h"

namespace HyperFast
{
	enum class VertexAttributeFlagBit : uint32_t
	{
		NONE		= 0U,
		POS3		= (1U << VERTEX_ATTRIB_LOCATION_POS),
		COLOR4		= (1U << VERTEX_ATTRIB_LOCATION_COLOR),
		NORMAL3		= (1U << VERTEX_ATTRIB_LOCATION_NORMAL),
	};

	enum class VertexAttributeFlag : uint32_t
	{
		POS3 = uint32_t(VertexAttributeFlagBit::POS3),
		
		POS3_COLOR4 =
		(
			uint32_t(VertexAttributeFlagBit::POS3) |
			uint32_t(VertexAttributeFlagBit::COLOR4)
		),

		POS3_NORMAL3 =
		(
			uint32_t(VertexAttributeFlagBit::POS3) |
			uint32_t(VertexAttributeFlagBit::NORMAL3)
		)
	};

	constexpr VertexAttributeFlag operator|(
		const VertexAttributeFlagBit lhs, const VertexAttributeFlagBit rhs)
	{
		return VertexAttributeFlag(uint32_t(lhs) | uint32_t(rhs));
	}

	constexpr VertexAttributeFlag operator|(
		const VertexAttributeFlag lhs, const VertexAttributeFlagBit rhs)
	{
		return VertexAttributeFlag(uint32_t(lhs) | uint32_t(rhs));
	}

	constexpr VertexAttributeFlag operator&(
		const VertexAttributeFlag lhs, const VertexAttributeFlag rhs)
	{
		return VertexAttributeFlag(uint32_t(lhs) & uint32_t(rhs));
	}

	constexpr VertexAttributeFlag operator~(const VertexAttributeFlagBit flagBit)
	{
		return VertexAttributeFlag(~uint32_t(flagBit));
	}

	constexpr VertexAttributeFlag &operator|=(
		VertexAttributeFlag &lhs, const VertexAttributeFlagBit rhs)
	{
		lhs = (lhs | rhs);
		return lhs;
	}

	constexpr VertexAttributeFlag &operator&=(
		VertexAttributeFlag &lhs, const VertexAttributeFlag rhs)
	{
		lhs = (lhs & rhs);
		return lhs;
	}

	constexpr bool operator&(const VertexAttributeFlag lhs, const VertexAttributeFlagBit rhs)
	{
		return (uint32_t(lhs) & uint32_t(rhs));
	}
}