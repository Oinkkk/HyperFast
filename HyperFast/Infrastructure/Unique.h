#pragma once

namespace Infra
{
	class Unique
	{
	public:
		Unique() = default;
		Unique(const Unique &) = delete;
		Unique(Unique &&) = delete;

		virtual ~Unique() noexcept = default;

		Unique &operator=(const Unique &) = delete;
		Unique &operator=(Unique &&) = delete;
	};
}
