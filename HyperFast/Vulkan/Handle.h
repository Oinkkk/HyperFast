#pragma once

#include "../Infrastructure/Unique.h"

namespace Vulkan
{
	template <typename $Type>
	class Handle : public Infra::Unique
	{
	public:
		constexpr Handle(const $Type handle) noexcept;

		[[nodiscard]]
		constexpr const $Type &getHandle() noexcept;

	protected:
		[[nodiscard]]
		constexpr const $Type &_getConstHandle() const noexcept;

	private:
		const $Type __handle;
	};

	template <typename $Type>
	constexpr Handle<$Type>::Handle(const $Type handle) noexcept :
		__handle{ handle }
	{}

	template <typename $Type>
	constexpr const $Type &Handle<$Type>::getHandle() noexcept
	{
		return __handle;
	}

	template <typename $Type>
	constexpr const $Type &Handle<$Type>::_getConstHandle() const noexcept
	{
		return __handle;
	}
}
