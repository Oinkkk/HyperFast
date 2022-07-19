#pragma once

#include <memory>
#include "../Vulkan/Semaphore.h"

namespace HyperFast
{
	class TimelineSemaphore : public Infra::Unique
	{
	public:
		TimelineSemaphore(Vulkan::Device &device, const uint64_t initialValue = 0ULL);

		constexpr void advance() noexcept;

		[[nodiscard]]
		constexpr uint64_t getValue() const noexcept;

		[[nodiscard]]
		VkSemaphore getHandle() const noexcept;

		VkResult signal() noexcept;
		VkResult wait(const uint64_t timeout) noexcept;
		VkResult wait(const uint64_t value, const uint64_t timeout) noexcept;

	private:
		Vulkan::Device &__device;

		std::unique_ptr<Vulkan::Semaphore> __pSemaphore;
		uint64_t __value;

		void __createSemaphore();
	};

	constexpr void TimelineSemaphore::advance() noexcept
	{
		__value++;
	}

	[[nodiscard]]
	constexpr uint64_t TimelineSemaphore::getValue() const noexcept
	{
		return __value;
	}
}
