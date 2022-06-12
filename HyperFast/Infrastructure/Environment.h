#pragma once

#include <taskflow/taskflow.hpp>

namespace Infra
{
	class Environment final
	{
	public:
		[[nodiscard]]
		constexpr tf::Executor &getTaskflowExecutor() noexcept;

		[[nodiscard]]
		static Environment &getInstance() noexcept;

	private:
		Environment() = default;
		tf::Executor __executor;
	};

	constexpr tf::Executor &Environment::getTaskflowExecutor() noexcept
	{
		return __executor;
	}
}