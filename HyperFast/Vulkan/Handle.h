#pragma once

#include "../Infrastructure/Deletable.h"

namespace Vulkan
{
	template <typename $Type>
	class Handle : public Infra::Deletable
	{
	public:
		Handle(const $Type handle) noexcept;
		virtual ~Handle() noexcept = default;

		[[nodiscard]]
		constexpr const $Type &getHandle() noexcept;

	protected:
		[[nodiscard]]
		constexpr const $Type &_getConstHandle() const noexcept;

	private:
		const $Type __handle;
	};

	template <typename $Type>
	Handle<$Type>::Handle(const $Type handle) noexcept :
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
