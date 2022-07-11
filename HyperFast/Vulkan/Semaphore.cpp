#include "Semaphore.h"
#include <exception>

namespace Vulkan
{
	Semaphore::Semaphore(Device &device, const VkSemaphoreCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	Semaphore::~Semaphore() noexcept
	{
		__destroy();
	}

	VkResult Semaphore::signal(const uint64_t value) noexcept
	{
		const VkSemaphoreSignalInfo info
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SIGNAL_INFO,
			.semaphore = getHandle(),
			.value = value
		};

		return __device.vkSignalSemaphore(&info);
	}

	VkResult Semaphore::wait(const uint64_t value, const uint64_t timeout) noexcept
	{
		const VkSemaphoreWaitInfo info
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
			.semaphoreCount = 1U,
			.pSemaphores = &(getHandle()),
			.pValues = &value
		};

		return __device.vkWaitSemaphores(&info, timeout);
	}

	VkResult Semaphore::vkGetSemaphoreCounterValue(uint64_t *const pValue) noexcept
	{
		return __device.vkGetSemaphoreCounterValue(getHandle(), nullptr);
	}

	void Semaphore::__destroy() noexcept
	{
		__device.vkDestroySemaphore(getHandle(), nullptr);
	}

	VkSemaphore Semaphore::__create(Device &device, const VkSemaphoreCreateInfo &createInfo)
	{
		VkSemaphore retVal{};
		device.vkCreateSemaphore(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkSemaphore." };

		return retVal;
	}
}