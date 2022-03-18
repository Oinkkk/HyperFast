#pragma once

#include <string>
#include <cstdio>

namespace Infra
{
	class StringFormatter
	{
	public:
		StringFormatter() = delete;

		template <typename ...$Args>
		static std::string format(const std::string_view &format, const $Args &...args) noexcept;

	private:
		static constexpr inline size_t BUFFER_SIZE{ 1024ULL };
	};

	template <typename ...$Args>
	static std::string StringFormatter::format(const std::string_view &format, const $Args &...args) noexcept
	{
		char buffer[BUFFER_SIZE]{};
		sprintf_s(buffer, format.data(), args...);

		return buffer;
	}
}
