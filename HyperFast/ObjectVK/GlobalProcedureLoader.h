#pragma once

#include "../Infrastructure/Unique.h"
#include "Procedure.h"

namespace ObjectVK
{
	class GlobalProcedureLoader : public Infra::Unique
	{
	public:
		void load();
		void free();

		[[nodiscard]]
		constexpr const GlobalProcedure &getProcedure() const noexcept;

		static GlobalProcedureLoader &getInstance() noexcept;

	private:
		HMODULE __loaderHandle{};
		GlobalProcedure __globalProc;

		GlobalProcedureLoader() = default;
		~GlobalProcedureLoader();

		void __loadProcedure(const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) noexcept;
	};

	constexpr const GlobalProcedure &GlobalProcedureLoader::getProcedure() const noexcept
	{
		return __globalProc;
	}
}