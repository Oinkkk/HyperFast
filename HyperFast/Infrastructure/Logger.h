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
		static constexpr inline std::string_view __severityTypeStrings[]
		{
			"VERBOSE", "INFO", "WARNING", "FATAL"
		};
	};
}
