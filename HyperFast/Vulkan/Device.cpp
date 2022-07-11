#include "Device.h"
#include <exception>

namespace Vulkan
{
	Device::Device(
		Instance &instance, PhysicalDevice &physicalDevice, const VkDeviceCreateInfo &createInfo) :
		Handle{ __create(physicalDevice, createInfo) },
		__instance{ instance }, __physicalDevice{physicalDevice}
	{
		__queryProc();
	}

	Device::~Device() noexcept
	{
		__destroy();
	}

	VkResult Device::vkDeviceWaitIdle() noexcept
	{
		return __proc.vkDeviceWaitIdle(getHandle());
	}

	void Device::vkGetDeviceQueue(
		const uint32_t queueFamilyIndex,
		const uint32_t queueIndex, VkQueue *const pQueue) noexcept
	{
		__proc.vkGetDeviceQueue(getHandle(), queueFamilyIndex, queueIndex, pQueue);
	}

	VkResult Device::vkQueueWaitIdle(const VkQueue queue) noexcept
	{
		return __proc.vkQueueWaitIdle(queue);
	}

	VkResult Device::vkQueueSubmit(
		const VkQueue queue, const uint32_t submitCount,
		const VkSubmitInfo *const pSubmits, const VkFence fence) noexcept
	{
		return __proc.vkQueueSubmit(queue, submitCount, pSubmits, fence);
	}

	VkResult Device::vkQueueSubmit2(
		const VkQueue queue, const uint32_t submitCount,
		const VkSubmitInfo2 *const pSubmits, const VkFence fence) noexcept
	{
		return __proc.vkQueueSubmit2(queue, submitCount, pSubmits, fence);
	}

	VkResult Device::vkQueuePresentKHR(
		const VkQueue queue, const VkPresentInfoKHR *const pPresentInfo) noexcept
	{
		return __proc.vkQueuePresentKHR(queue, pPresentInfo);
	}

	VkResult Device::vkCreateCommandPool(
		const VkCommandPoolCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkCommandPool *const pCommandPool) noexcept
	{
		return __proc.vkCreateCommandPool(getHandle(), pCreateInfo, pAllocator, pCommandPool);
	}

	void Device::vkDestroyCommandPool(
		const VkCommandPool commandPool, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyCommandPool(getHandle(), commandPool, pAllocator);
	}

	VkResult Device::vkResetCommandPool(
		const VkCommandPool commandPool, const VkCommandPoolResetFlags flags) noexcept
	{
		return __proc.vkResetCommandPool(getHandle(), commandPool, flags);
	}

	VkResult Device::vkAllocateCommandBuffers(
		const VkCommandBufferAllocateInfo *const pAllocateInfo,
		VkCommandBuffer *const pCommandBuffers) noexcept
	{
		return __proc.vkAllocateCommandBuffers(getHandle(), pAllocateInfo, pCommandBuffers);
	}

	VkResult Device::vkCreateSwapchainKHR(
		const VkSwapchainCreateInfoKHR *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkSwapchainKHR *const pSwapchain) noexcept
	{
		return __proc.vkCreateSwapchainKHR(getHandle(), pCreateInfo, pAllocator, pSwapchain);
	}

	void Device::vkDestroySwapchainKHR(
		const VkSwapchainKHR swapchain, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		return __proc.vkDestroySwapchainKHR(getHandle(), swapchain, pAllocator);
	}

	VkResult Device::vkGetSwapchainImagesKHR(
		const VkSwapchainKHR swapchain, uint32_t *const pSwapchainImageCount,
		VkImage *const pSwapchainImages) noexcept
	{
		return __proc.vkGetSwapchainImagesKHR(
			getHandle(), swapchain, pSwapchainImageCount, pSwapchainImages);
	}

	VkResult Device::vkAcquireNextImageKHR(
		const VkSwapchainKHR swapchain, const uint64_t timeout,
		const VkSemaphore semaphore, const VkFence fence, uint32_t *const pImageIndex) noexcept
	{
		return __proc.vkAcquireNextImageKHR(
			getHandle(), swapchain, timeout, semaphore, fence, pImageIndex);
	}

	VkResult Device::vkAcquireNextImage2KHR(
		const VkAcquireNextImageInfoKHR *const pAcquireInfo, uint32_t *const pImageIndex) noexcept
	{
		return __proc.vkAcquireNextImage2KHR(getHandle(), pAcquireInfo, pImageIndex);
	}

	VkResult Device::vkCreateImageView(
		const VkImageViewCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkImageView *const pView) noexcept
	{
		return __proc.vkCreateImageView(getHandle(), pCreateInfo, pAllocator, pView);
	}

	void Device::vkDestroyImageView(
		const VkImageView imageView, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyImageView(getHandle(), imageView, pAllocator);
	}

	VkResult Device::vkCreateFramebuffer(
		const VkFramebufferCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkFramebuffer *const pFramebuffer) noexcept
	{
		return __proc.vkCreateFramebuffer(getHandle(), pCreateInfo, pAllocator, pFramebuffer);
	}

	void Device::vkDestroyFramebuffer(
		const VkFramebuffer framebuffer, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyFramebuffer(getHandle(), framebuffer, pAllocator);
	}

	VkResult Device::vkCreateShaderModule(
		const VkShaderModuleCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkShaderModule *const pShaderModule) noexcept
	{
		return __proc.vkCreateShaderModule(getHandle(), pCreateInfo, pAllocator, pShaderModule);
	}

	void Device::vkDestroyShaderModule(
		const VkShaderModule shaderModule, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyShaderModule(getHandle(), shaderModule, pAllocator);
	}

	VkResult Device::vkCreateRenderPass2(
		const VkRenderPassCreateInfo2 *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkRenderPass *const pRenderPass) noexcept
	{
		return __proc.vkCreateRenderPass2(getHandle(), pCreateInfo, pAllocator, pRenderPass);
	}

	void Device::vkDestroyRenderPass(
		const VkRenderPass renderPass, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyRenderPass(getHandle(), renderPass, pAllocator);
	}

	VkResult Device::vkCreatePipelineLayout(
		const VkPipelineLayoutCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkPipelineLayout *const pPipelineLayout) noexcept
	{
		return __proc.vkCreatePipelineLayout(getHandle(), pCreateInfo, pAllocator, pPipelineLayout);
	}

	void Device::vkDestroyPipelineLayout(
		const VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyPipelineLayout(getHandle(), pipelineLayout, pAllocator);
	}

	VkResult Device::vkCreatePipelineCache(
		const VkPipelineCacheCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkPipelineCache *const pPipelineCache) noexcept
	{
		return __proc.vkCreatePipelineCache(getHandle(), pCreateInfo, pAllocator, pPipelineCache);
	}

	void Device::vkDestroyPipelineCache(
		const VkPipelineCache pipelineCache, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyPipelineCache(getHandle(), pipelineCache, pAllocator);
	}
	VkResult Device::vkCreateGraphicsPipelines(
		const VkPipelineCache pipelineCache, const uint32_t createInfoCount,
		const VkGraphicsPipelineCreateInfo *const pCreateInfos,
		const VkAllocationCallbacks *const pAllocator, VkPipeline *const pPipelines) noexcept
	{
		return __proc.vkCreateGraphicsPipelines(
			getHandle(), pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
	}

	void Device::vkDestroyPipeline(
		const VkPipeline pipeline, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyPipeline(getHandle(), pipeline, pAllocator);
	}

	VkResult Device::vkCreateSemaphore(
		const VkSemaphoreCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkSemaphore *const pSemaphore) noexcept
	{
		return __proc.vkCreateSemaphore(getHandle(), pCreateInfo, pAllocator, pSemaphore);
	}

	void Device::vkDestroySemaphore(
		const VkSemaphore semaphore, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroySemaphore(getHandle(), semaphore, pAllocator);
	}

	VkResult Device::vkSignalSemaphore(
		const VkSemaphoreSignalInfo *const pSignalInfo) noexcept
	{
		return __proc.vkSignalSemaphore(getHandle(), pSignalInfo);
	}

	VkResult Device::vkWaitSemaphores(
		const VkSemaphoreWaitInfo *const pWaitInfo, const uint64_t timeout) noexcept
	{
		return __proc.vkWaitSemaphores(getHandle(), pWaitInfo, timeout);
	}

	VkResult Device::vkGetSemaphoreCounterValue(
		const VkSemaphore semaphore, uint64_t *const pValue) noexcept
	{
		return __proc.vkGetSemaphoreCounterValue(getHandle(), semaphore, pValue);
	}

	VkResult Device::vkCreateFence(
		const VkFenceCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkFence *const pFence) noexcept
	{
		return __proc.vkCreateFence(getHandle(), pCreateInfo, pAllocator, pFence);
	}

	void Device::vkDestroyFence(
		const VkFence fence, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyFence(getHandle(), fence, pAllocator);
	}

	VkResult Device::vkWaitForFences(
		const uint32_t fenceCount, const VkFence *const pFences,
		const VkBool32 waitAll, const uint64_t timeout) noexcept
	{
		return __proc.vkWaitForFences(getHandle(), fenceCount, pFences, waitAll, timeout);
	}

	VkResult Device::vkResetFences(
		const uint32_t fenceCount, const VkFence *const pFences) noexcept
	{
		return __proc.vkResetFences(getHandle(), fenceCount, pFences);
	}

	VkResult Device::vkCreateBuffer(
		const VkBufferCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkBuffer *const pBuffer) noexcept
	{
		return __proc.vkCreateBuffer(getHandle(), pCreateInfo, pAllocator, pBuffer);
	}

	void Device::vkDestroyBuffer(
		const VkBuffer buffer, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyBuffer(getHandle(), buffer, pAllocator);
	}

	void Device::vkGetBufferMemoryRequirements(
		const VkBuffer buffer, VkMemoryRequirements *const pMemoryRequirements) noexcept
	{
		__proc.vkGetBufferMemoryRequirements(getHandle(), buffer, pMemoryRequirements);
	}

	VkResult Device::vkBindBufferMemory(
		const VkBuffer buffer, const VkDeviceMemory memory, const VkDeviceSize memoryOffset) noexcept
	{
		return __proc.vkBindBufferMemory(getHandle(), buffer, memory, memoryOffset);
	}

	VkResult Device::vkAllocateMemory(
		const VkMemoryAllocateInfo *const pAllocateInfo,
		const VkAllocationCallbacks *const pAllocator, VkDeviceMemory *const pMemory) noexcept
	{
		return __proc.vkAllocateMemory(getHandle(), pAllocateInfo, pAllocator, pMemory);
	}

	void Device::vkFreeMemory(
		const VkDeviceMemory memory, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkFreeMemory(getHandle(), memory, pAllocator);
	}

	VkResult Device::vkMapMemory(
		const VkDeviceMemory memory, const VkDeviceSize offset,
		const VkDeviceSize size, const VkMemoryMapFlags flags, void **const ppData) noexcept
	{
		return __proc.vkMapMemory(getHandle(), memory, offset, size, flags, ppData);
	}

	void Device::vkUnmapMemory(VkDeviceMemory memory) noexcept
	{
		__proc.vkUnmapMemory(getHandle(), memory);
	}

	VkResult Device::vkBeginCommandBuffer(
		const VkCommandBuffer commandBuffer,
		const VkCommandBufferBeginInfo *const pBeginInfo) noexcept
	{
		return __proc.vkBeginCommandBuffer(commandBuffer, pBeginInfo);
	}

	VkResult Device::vkEndCommandBuffer(const VkCommandBuffer commandBuffer) noexcept
	{
		return __proc.vkEndCommandBuffer(commandBuffer);
	}

	VkResult Device::vkResetCommandBuffer(
		const VkCommandBuffer commandBuffer, const VkCommandBufferResetFlags flags) noexcept
	{
		return __proc.vkResetCommandBuffer(commandBuffer, flags);
	}

	void Device::vkCmdBeginRenderPass(
		const VkCommandBuffer commandBuffer,
		const VkRenderPassBeginInfo *const pRenderPassBegin, const VkSubpassContents contents) noexcept
	{
		__proc.vkCmdBeginRenderPass(commandBuffer, pRenderPassBegin, contents);
	}

	void Device::vkCmdEndRenderPass(const VkCommandBuffer commandBuffer) noexcept
	{
		__proc.vkCmdEndRenderPass(commandBuffer);
	}

	void Device::vkCmdBindPipeline(
		const VkCommandBuffer commandBuffer,
		const VkPipelineBindPoint pipelineBindPoint, const VkPipeline pipeline) noexcept
	{
		__proc.vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
	}

	void Device::vkCmdBindVertexBuffers(
		const VkCommandBuffer commandBuffer, const uint32_t firstBinding,
		const uint32_t bindingCount, const VkBuffer *const pBuffers,
		const VkDeviceSize *const pOffsets) noexcept
	{
		__proc.vkCmdBindVertexBuffers(
			commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
	}

	void Device::vkCmdBindIndexBuffer(
		const VkCommandBuffer commandBuffer, const VkBuffer buffer,
		const VkDeviceSize offset, const VkIndexType indexType) noexcept
	{
		__proc.vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
	}

	void Device::vkCmdDrawIndexedIndirectCount(
		const VkCommandBuffer commandBuffer, const VkBuffer buffer,
		const VkDeviceSize offset, const VkBuffer countBuffer,
		const VkDeviceSize countBufferOffset, const uint32_t maxDrawCount,
		const uint32_t stride) noexcept
	{
		__proc.vkCmdDrawIndexedIndirectCount(
			commandBuffer, buffer, offset, countBuffer,
			countBufferOffset, maxDrawCount, stride);
	}

	void Device::vkCmdPipelineBarrier2(
		const VkCommandBuffer commandBuffer, const VkDependencyInfo *const pDependencyInfo) noexcept
	{
		__proc.vkCmdPipelineBarrier2(commandBuffer, pDependencyInfo);
	}

	void Device::vkCmdCopyBuffer(
		const VkCommandBuffer commandBuffer, const VkBuffer srcBuffer,
		const VkBuffer dstBuffer, const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept
	{
		__proc.vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
	}

	void Device::__queryProc() noexcept
	{
		__proc = __instance.queryDeviceProcedure(getHandle());
	}

	void Device::__destroy() noexcept
	{
		__proc.vkDestroyDevice(getHandle(), nullptr);
	}

	VkDevice Device::__create(PhysicalDevice &physicalDevice, const VkDeviceCreateInfo &createInfo)
	{
		VkDevice retVal{};
		physicalDevice.vkCreateDevice(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkDevice." };

		return retVal;
	}
}