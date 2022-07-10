#include "Queue.h"

namespace Vulkan
{
	Queue::Queue(
		Device &device, const uint32_t queueFamilyIndex, const uint32_t queueIndex) noexcept :
		Handle{ __retrieve(device, queueFamilyIndex, queueIndex) }, __device{ device }
	{}

	VkResult Queue::vkQueueWaitIdle() noexcept
	{
		return __device.vkQueueWaitIdle(getHandle());
	}

	VkResult Queue::vkQueueSubmit2(
		const uint32_t submitCount,
		const VkSubmitInfo2 *const pSubmits, const VkFence fence) noexcept
	{
		return __device.vkQueueSubmit2(getHandle(), submitCount, pSubmits, fence);
	}

	VkResult Queue::vkQueuePresentKHR(const VkPresentInfoKHR *const pPresentInfo) noexcept
	{
		return __device.vkQueuePresentKHR(getHandle(), pPresentInfo);
	}

	VkQueue Queue::__retrieve(
		Device &device, const uint32_t queueFamilyIndex, const uint32_t queueIndex) noexcept
	{
		VkQueue retVal{};
		device.vkGetDeviceQueue(queueFamilyIndex, queueIndex, &retVal);

		return retVal;
	}
}
