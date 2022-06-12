#pragma once

#include <cstdint>

namespace HyperFast
{
	enum class VertexAttributeFlagBit : uint32_t
	{
		NONE		= 0U,

		// vec3
		POS			= 0b0001,

		// vec4
		COLOR		= 0b0010
	};

	enum class VertexAttributeFlag : uint32_t
	{
		POS = uint32_t(VertexAttributeFlagBit::POS),

		POS_COLOR =
		(
			uint32_t(VertexAttributeFlagBit::POS) |
			uint32_t(VertexAttributeFlagBit::COLOR)
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

	constexpr VertexAttributeFlag &operator|=(
		VertexAttributeFlag &lhs, const VertexAttributeFlagBit rhs)
	{
		lhs = VertexAttributeFlag(uint32_t(lhs) | uint32_t(rhs));
		return lhs;
	}

	constexpr bool operator&(const VertexAttributeFlag lhs, const VertexAttributeFlagBit rhs)
	{
		return (uint32_t(lhs) & uint32_t(rhs));
	}
}