#pragma once

#include "Unique.h"
#include "LogSeverityType.h"
#include <mutex>

namespace Infra
{
	class Logger : public Unique
	{
	public:
		Logger() = default;
		void log(const LogSeverityType severityType, const std::string_view &message) noexcept;

	private:
		std::mutex __mutex;

		static constexpr inline std::string_view __severityTypeColors[]
		{
			"", "\u001b[36;1m", "\u001b[33;1m", "\u001b[31;1m"
		};

		static constexpr inline std::string_view __resetColor{ "\u001b[0m" };

		static constexpr inline std::string_view __severityTypeStrings[]
		{
			"VERBOSE", "INFO", "WARNING", "FATAL"
		};
	};
}
