#include "TimelineSemaphore.h"

namespace HyperFast
{
	TimelineSemaphore::TimelineSemaphore(Vulkan::Device &device, const uint64_t initialValue) :
		__device{ device }, __value { initialValue }
	{
		__createSemaphore();
	}

	VkSemaphore TimelineSemaphore::getHandle() const noexcept
	{
		return __pSemaphore->getHandle();
	}

	VkResult TimelineSemaphore::signal() noexcept
	{
		return __pSemaphore->signal(__value);
	}

	VkResult TimelineSemaphore::wait(const uint64_t timeout) noexcept
	{
		return __pSemaphore->wait(__value, timeout);
	}

	VkResult TimelineSemaphore::wait(const uint64_t value, const uint64_t timeout) noexcept
	{
		return __pSemaphore->wait(value, timeout);
	}

	void TimelineSemaphore::__createSemaphore()
	{
		const VkSemaphoreTypeCreateInfo timelineInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE,
			.initialValue = __value
		};

		const VkSemaphoreCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timelineInfo
		};

		__pSemaphore = std::make_unique<Vulkan::Semaphore>(__device, createInfo);
	}
}