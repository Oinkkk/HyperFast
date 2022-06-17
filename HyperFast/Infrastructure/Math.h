#pragma once

#include <type_traits>

namespace Infra
{
	class Math
	{
	public:
		template <typename $Int, typename std::enable_if_t<std::is_integral_v<$Int>>* = nullptr>
		[[nodiscard]]
		static constexpr $Int floorAlign(const $Int source, const $Int alignment) noexcept;

		template <typename $Int, typename std::enable_if_t<std::is_integral_v<$Int>>* = nullptr>
		[[nodiscard]]
		static constexpr $Int ceilAlign(const $Int source, const $Int alignment) noexcept;

		template <typename $Int, typename std::enable_if_t<std::is_integral_v<$Int>>* = nullptr>
		[[nodiscard]]
		static constexpr $Int calcGCD(const $Int lhs, const $Int rhs) noexcept;

		template <typename $Int, typename std::enable_if_t<std::is_integral_v<$Int>>* = nullptr>
		[[nodiscard]]
		static constexpr $Int calcLCM(const $Int lhs, const $Int rhs) noexcept;

	private:
		Math() = delete;
	};

	template <typename $Int, typename std::enable_if_t<std::is_integral_v<$Int>> *>
	static constexpr $Int Math::floorAlign(const $Int source, const $Int alignment) noexcept
	{
		return (source - (source % alignment));
	}

	template <typename $Int, typename std::enable_if_t<std::is_integral_v<$Int>> *>
	static constexpr $Int Math::ceilAlign(const $Int source, const $Int alignment) noexcept
	{
		const $Int remainder{ source % alignment };
		if (!remainder)
			return source;

		return ((source + alignment) - remainder);
	}

	template <typename $Int, typename std::enable_if_t<std::is_integral_v<$Int>> *>
	static constexpr $Int Math::calcGCD($Int lhs, $Int rhs) noexcept
	{
		while (rhs)
		{
			const $Int temp{ rhs };
			rhs = (lhs % rhs);
			lhs = temp;
		}

		return lhs;
	}

	template <typename $Int, typename std::enable_if_t<std::is_integral_v<$Int>> *>
	static constexpr $Int Math::calcLCM(const $Int lhs, const $Int rhs) noexcept
	{
		return ((lhs * rhs) / calcGCD(lhs, rhs));
	}
}
