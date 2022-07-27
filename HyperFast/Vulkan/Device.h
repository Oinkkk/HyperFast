#pragma once

#include "PhysicalDevice.h"

namespace Vulkan
{
	class Device final : public Handle<VkDevice>
	{
	public:
		Device(
			Instance &instance, PhysicalDevice &physicalDevice,
			const VkDeviceCreateInfo &createInfo);

		~Device() noexcept;

		// Device
		VkResult vkDeviceWaitIdle() noexcept;

		// Queue
		void vkGetDeviceQueue(
			const uint32_t queueFamilyIndex,
			const uint32_t queueIndex, VkQueue *const pQueue) noexcept;

		VkResult vkQueueWaitIdle(const VkQueue queue) noexcept;
		
		VkResult vkQueueSubmit(
			const VkQueue queue, const uint32_t submitCount,
			const VkSubmitInfo *const pSubmits, const VkFence fence) noexcept;
		
		VkResult vkQueueSubmit2(
			const VkQueue queue, const uint32_t submitCount,
			const VkSubmitInfo2 *const pSubmits, const VkFence fence) noexcept;
		
		VkResult vkQueuePresentKHR(
			const VkQueue queue, const VkPresentInfoKHR *const pPresentInfo) noexcept;

		// Command pool
		VkResult vkCreateCommandPool(
			const VkCommandPoolCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkCommandPool *const pCommandPool) noexcept;
		
		void vkDestroyCommandPool(
			const VkCommandPool commandPool, const VkAllocationCallbacks *const pAllocator) noexcept;
		
		VkResult vkResetCommandPool(
			const VkCommandPool commandPool, const VkCommandPoolResetFlags flags) noexcept;

		VkResult vkAllocateCommandBuffers(
			const VkCommandBufferAllocateInfo *const pAllocateInfo,
			VkCommandBuffer *const pCommandBuffers) noexcept;
		
		// Swapchain
		VkResult vkCreateSwapchainKHR(
			const VkSwapchainCreateInfoKHR *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkSwapchainKHR *const pSwapchain) noexcept;

		void vkDestroySwapchainKHR(
			const VkSwapchainKHR swapchain, const VkAllocationCallbacks *const pAllocator) noexcept;

		VkResult vkGetSwapchainImagesKHR(
			const VkSwapchainKHR swapchain, uint32_t *const pSwapchainImageCount,
			VkImage *const pSwapchainImages) noexcept;

		VkResult vkAcquireNextImageKHR(
			const VkSwapchainKHR swapchain, const uint64_t timeout,
			const VkSemaphore semaphore, const VkFence fence, uint32_t *const pImageIndex) noexcept;

		VkResult vkAcquireNextImage2KHR(
			const VkAcquireNextImageInfoKHR *const pAcquireInfo, uint32_t *const pImageIndex) noexcept;

		// ImageView
		VkResult vkCreateImageView(
			const VkImageViewCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkImageView *const pView) noexcept;

		void vkDestroyImageView(
			const VkImageView imageView, const VkAllocationCallbacks *const pAllocator) noexcept;

		// Framebuffer
		VkResult vkCreateFramebuffer(
			const VkFramebufferCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkFramebuffer *const pFramebuffer) noexcept;

		void vkDestroyFramebuffer(
			const VkFramebuffer framebuffer, const VkAllocationCallbacks *const pAllocator) noexcept;

		// ShaderModule
		VkResult vkCreateShaderModule(
			const VkShaderModuleCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkShaderModule *const pShaderModule) noexcept;
		
		void vkDestroyShaderModule(
			const VkShaderModule shaderModule, const VkAllocationCallbacks *const pAllocator) noexcept;

		// Render pass
		VkResult vkCreateRenderPass2(
			const VkRenderPassCreateInfo2 *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkRenderPass *const pRenderPass) noexcept;

		void vkDestroyRenderPass(
			const VkRenderPass renderPass, const VkAllocationCallbacks *const pAllocator) noexcept;

		// Pipeline layout
		VkResult vkCreatePipelineLayout(
			const VkPipelineLayoutCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkPipelineLayout *const pPipelineLayout) noexcept;
		
		void vkDestroyPipelineLayout(
			const VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *const pAllocator) noexcept;

		// Pipeline cache
		VkResult vkCreatePipelineCache(
			const VkPipelineCacheCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkPipelineCache *const pPipelineCache) noexcept;
		
		void vkDestroyPipelineCache(
			const VkPipelineCache pipelineCache, const VkAllocationCallbacks *const pAllocator) noexcept;

		// Pipeline
		VkResult vkCreateGraphicsPipelines(
			const VkPipelineCache pipelineCache, const uint32_t createInfoCount,
			const VkGraphicsPipelineCreateInfo *const pCreateInfos,
			const VkAllocationCallbacks *const pAllocator, VkPipeline *const pPipelines) noexcept;

		void vkDestroyPipeline(
			const VkPipeline pipeline, const VkAllocationCallbacks *const pAllocator) noexcept;

		// Semaphore
		VkResult vkCreateSemaphore(
			const VkSemaphoreCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkSemaphore *const pSemaphore) noexcept;

		void vkDestroySemaphore(
			const VkSemaphore semaphore, const VkAllocationCallbacks *const pAllocator) noexcept;

		VkResult vkSignalSemaphore(
			const VkSemaphoreSignalInfo *const pSignalInfo) noexcept;

		VkResult vkWaitSemaphores(
			const VkSemaphoreWaitInfo *const pWaitInfo, const uint64_t timeout) noexcept;

		VkResult vkGetSemaphoreCounterValue(
			const VkSemaphore semaphore, uint64_t *const pValue) noexcept;

		// Fence
		VkResult vkCreateFence(
			const VkFenceCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkFence *const pFence) noexcept;

		void vkDestroyFence(
			const VkFence fence, const VkAllocationCallbacks *const pAllocator) noexcept;

		VkResult vkWaitForFences(
			const uint32_t fenceCount, const VkFence *const pFences,
			const VkBool32 waitAll, const uint64_t timeout) noexcept;

		VkResult vkResetFences(
			const uint32_t fenceCount, const VkFence *const pFences) noexcept;

		// Buffer
		VkResult vkCreateBuffer(
			const VkBufferCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkBuffer *const pBuffer) noexcept;

		void vkDestroyBuffer(
			const VkBuffer buffer, const VkAllocationCallbacks *const pAllocator) noexcept;

		void vkGetBufferMemoryRequirements(
			const VkBuffer buffer, VkMemoryRequirements *const pMemoryRequirements) noexcept;

		VkResult vkBindBufferMemory(
			const VkBuffer buffer, const VkDeviceMemory memory, const VkDeviceSize memoryOffset) noexcept;

		// Memory
		VkResult vkAllocateMemory(
			const VkMemoryAllocateInfo *const pAllocateInfo,
			const VkAllocationCallbacks *const pAllocator, VkDeviceMemory *const pMemory) noexcept;

		void vkFreeMemory(
			const VkDeviceMemory memory, const VkAllocationCallbacks *const pAllocator) noexcept;

		VkResult vkMapMemory(
			const VkDeviceMemory memory, const VkDeviceSize offset,
			const VkDeviceSize size, const VkMemoryMapFlags flags, void **const ppData) noexcept;

		void vkUnmapMemory(VkDeviceMemory memory) noexcept;

		// DescriptorSetLayout
		VkResult vkCreateDescriptorSetLayout(
			const VkDescriptorSetLayoutCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator,
			VkDescriptorSetLayout *const pSetLayout) noexcept;

		void vkDestroyDescriptorSetLayout(
			const VkDescriptorSetLayout descriptorSetLayout,
			const VkAllocationCallbacks *const pAllocator) noexcept;

		// Command buffer
		VkResult vkBeginCommandBuffer(
			const VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *const pBeginInfo) noexcept;

		VkResult vkEndCommandBuffer(const VkCommandBuffer commandBuffer) noexcept;
		
		VkResult vkResetCommandBuffer(
			const VkCommandBuffer commandBuffer,
			const VkCommandBufferResetFlags flags) noexcept;

		void vkCmdBeginRenderPass(
			const VkCommandBuffer commandBuffer,
			const VkRenderPassBeginInfo *const pRenderPassBegin, const VkSubpassContents contents) noexcept;

		void vkCmdEndRenderPass(const VkCommandBuffer commandBuffer) noexcept;

		void vkCmdBindPipeline(
			const VkCommandBuffer commandBuffer,
			const VkPipelineBindPoint pipelineBindPoint, const VkPipeline pipeline) noexcept;

		void vkCmdBindVertexBuffers(
			const VkCommandBuffer commandBuffer, const uint32_t firstBinding,
			const uint32_t bindingCount, const VkBuffer *const pBuffers,
			const VkDeviceSize *const pOffsets) noexcept;

		void vkCmdBindIndexBuffer(
			const VkCommandBuffer commandBuffer, const VkBuffer buffer,
			const VkDeviceSize offset, const VkIndexType indexType) noexcept;

		void vkCmdDrawIndexedIndirectCount(
			const VkCommandBuffer commandBuffer, const VkBuffer buffer,
			const VkDeviceSize offset, const VkBuffer countBuffer,
			const VkDeviceSize countBufferOffset, const uint32_t maxDrawCount,
			const uint32_t stride) noexcept;
		
		void vkCmdPipelineBarrier2(
			const VkCommandBuffer commandBuffer, const VkDependencyInfo *const pDependencyInfo) noexcept;

		void vkCmdCopyBuffer(
			const VkCommandBuffer commandBuffer, const VkBuffer srcBuffer,
			const VkBuffer dstBuffer, const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept;

		void vkCmdExecuteCommands(
			const VkCommandBuffer commandBuffer,
			const uint32_t commandBufferCount, const VkCommandBuffer *const pCommandBuffers) noexcept;

	private:
		Instance &__instance;
		PhysicalDevice &__physicalDevice;
		DeviceProcedure __proc{};

		void __queryProc() noexcept;
		void __destroy() noexcept;

		[[nodiscard]]
		static VkDevice __create(PhysicalDevice &physicalDevice, const VkDeviceCreateInfo &createInfo);
	};
}
