#pragma once

#include "Unique.h"
#include <chrono>

namespace Infra
{
	template <typename $Precision = float>
	class Timer : public Unique
	{
	public:
		void start() noexcept;
		void end() noexcept;
		constexpr void reset() noexcept;

		[[nodiscard]]
		constexpr $Precision getElapsed() const noexcept;

	private:
		bool __initialized{};
		std::chrono::steady_clock::time_point __from{};
		std::chrono::steady_clock::time_point __to{};
	};

	template <typename $Precision>
	void Timer<$Precision>::start() noexcept
	{
		__from = std::chrono::steady_clock::now();
		__initialized = true;
	}

	template <typename $Precision>
	void Timer<$Precision>::end() noexcept
	{
		__to = std::chrono::steady_clock::now();
	}

	template <typename $Precision>
	constexpr void Timer<$Precision>::reset() noexcept
	{
		__initialized = false;
	}

	template <typename $Precision>
	constexpr $Precision Timer<$Precision>::getElapsed() const noexcept
	{
		if (!__initialized)
			return {};

		const int64_t deltaTimeInt
		{
			std::chrono::duration_cast<std::chrono::nanoseconds>(__to - __from).count()
		};

		return $Precision(deltaTimeInt * 1e-6);
	}
}
