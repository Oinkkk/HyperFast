#include "Logger.h"
#include <iostream>
#include <sstream>
#include <chrono>

namespace Infra
{
	void Logger::log(const LogSeverityType severityType, const std::string_view &message) noexcept
	{
		std::ostringstream oss;

		const std::string_view &severity{ __severityTypeStrings[int(severityType)] };
		oss << '[' << severity << "][";

		const auto &curTime{ std::chrono::system_clock::now() };
		const std::time_t curTimeAsTimeT{ std::chrono::system_clock::to_time_t(curTime) };

		tm curTimeAsTM;
		localtime_s(&curTimeAsTM, &curTimeAsTimeT);

		oss << std::put_time(&curTimeAsTM, "%T") << "]";
		oss << message;

		std::lock_guard lck{ __mutex };
		std::cout << oss.str() << std::endl;
	}
}