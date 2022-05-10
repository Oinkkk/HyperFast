#pragma once

#include "../Infrastructure/Unique.h"
#include <Windows.h>

namespace ObjectVK
{
	class GlobalProcedure : public Infra::Unique
	{
	public:
		void load();
		void free();

		static GlobalProcedure &getInstance() noexcept;

	private:
		HMODULE __loaderHandle{};
		GlobalProcedure() = default;
		~GlobalProcedure();
	};
}