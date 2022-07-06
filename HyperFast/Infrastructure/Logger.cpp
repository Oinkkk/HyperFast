#include "Logger.h"
#include <iostream>
#include <sstream>
#include <chrono>

namespace Infra
{
	void Logger::log(const LogSeverityType severityType, const std::string_view &message) noexcept
	{
		const int severityTypeInt{ int(severityType) };
		const std::string_view &severityColor{ __severityTypeColors[severityTypeInt] };
		const std::string_view &severityString{ __severityTypeStrings[severityTypeInt] };

		std::ostringstream oss;
		oss << severityColor;
		oss << '[' << severityString << "][";

		const auto &curTime{ std::chrono::system_clock::now() };
		const std::time_t curTimeAsTimeT{ std::chrono::system_clock::to_time_t(curTime) };

		tm curTimeAsTM;
		localtime_s(&curTimeAsTM, &curTimeAsTimeT);

		oss << std::put_time(&curTimeAsTM, "%T") << "] ";
		oss << message;
		oss << __resetColor;

		std::lock_guard lck{ __mutex };
		std::cerr << oss.str() << std::endl;
	}
}