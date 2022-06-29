#include "VulkanLoader.h"
#include <exception>
#include <cassert>

namespace VKL
{
	VulkanLoader::~VulkanLoader()
	{
		try
		{
			free();
		}
		catch (...)
		{
		}
	}

	void VulkanLoader::load()
	{
		if (__handle)
			return;

		__handle = LoadLibrary("vulkan-1.dll");
		if (!__handle)
			throw std::exception{ "Cannot find a vulkan loader." };

		const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
		{
			reinterpret_cast<PFN_vkGetInstanceProcAddr>(
				GetProcAddress(__handle, "vkGetInstanceProcAddr"))
		};

		__loadGlobalProc(vkGetInstanceProcAddr);
	}

	void VulkanLoader::free()
	{
		if (!__handle)
			return;

		if (!FreeLibrary(__handle))
			throw std::exception{ "Cannot free a vulkan loader." };

		__handle = nullptr;
	}

	InstanceProcedure VulkanLoader::queryInstanceProcedure(const VkInstance instance) noexcept
	{
		assert(__handle);

		InstanceProcedure retVal;

		retVal.vkDestroyInstance =
			reinterpret_cast<PFN_vkDestroyInstance>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkDestroyInstance"));

		retVal.vkCreateDebugUtilsMessengerEXT =
			reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

		retVal.vkDestroyDebugUtilsMessengerEXT =
			reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

		retVal.vkEnumeratePhysicalDevices =
			reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));

		retVal.vkEnumeratePhysicalDeviceGroups =
			reinterpret_cast<PFN_vkEnumeratePhysicalDeviceGroups>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroups"));

		retVal.vkGetPhysicalDeviceProperties =
			reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties"));

		retVal.vkGetPhysicalDeviceProperties2 =
			reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2"));

		retVal.vkGetPhysicalDeviceQueueFamilyProperties =
			reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties"));

		retVal.vkGetPhysicalDeviceQueueFamilyProperties2 =
			reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2"));

		retVal.vkGetPhysicalDeviceMemoryProperties2 =
			reinterpret_cast<PFN_vkGetPhysicalDeviceMemoryProperties2>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceMemoryProperties2"));

		retVal.vkGetPhysicalDeviceWin32PresentationSupportKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));

		retVal.vkCreateWin32SurfaceKHR =
			reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));

		retVal.vkDestroySurfaceKHR =
			reinterpret_cast<PFN_vkDestroySurfaceKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR"));

		retVal.vkGetPhysicalDeviceSurfaceSupportKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));

		retVal.vkGetPhysicalDeviceSurfaceCapabilitiesKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));

		retVal.vkGetPhysicalDeviceSurfaceFormatsKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));

		retVal.vkGetPhysicalDeviceSurfacePresentModesKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

		retVal.vkCreateDevice =
			reinterpret_cast<PFN_vkCreateDevice>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkCreateDevice"));

		retVal.vkGetDeviceProcAddr =
			reinterpret_cast<PFN_vkGetDeviceProcAddr>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr"));

		return retVal;
	}

	DeviceProcedure VulkanLoader::queryDeviceProcedure(
		const PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr, const VkDevice device) noexcept
	{
		assert(__handle);

		DeviceProcedure retVal;

		retVal.vkDestroyDevice =
			reinterpret_cast<PFN_vkDestroyDevice>(
				vkGetDeviceProcAddr(device, "vkDestroyDevice"));

		retVal.vkDeviceWaitIdle =
			reinterpret_cast<PFN_vkDeviceWaitIdle>(
				vkGetDeviceProcAddr(device, "vkDeviceWaitIdle"));

		retVal.vkGetDeviceQueue =
			reinterpret_cast<PFN_vkGetDeviceQueue>(
				vkGetDeviceProcAddr(device, "vkGetDeviceQueue"));

		retVal.vkQueueWaitIdle =
			reinterpret_cast<PFN_vkQueueWaitIdle>(
				vkGetDeviceProcAddr(device, "vkQueueWaitIdle"));

		retVal.vkQueueSubmit =
			reinterpret_cast<PFN_vkQueueSubmit>(
				vkGetDeviceProcAddr(device, "vkQueueSubmit"));

		retVal.vkQueueSubmit2 =
			reinterpret_cast<PFN_vkQueueSubmit2>(
				vkGetDeviceProcAddr(device, "vkQueueSubmit2"));

		retVal.vkQueuePresentKHR =
			reinterpret_cast<PFN_vkQueuePresentKHR>(
				vkGetDeviceProcAddr(device, "vkQueuePresentKHR"));

		retVal.vkCreateCommandPool =
			reinterpret_cast<PFN_vkCreateCommandPool>(
				vkGetDeviceProcAddr(device, "vkCreateCommandPool"));

		retVal.vkDestroyCommandPool =
			reinterpret_cast<PFN_vkDestroyCommandPool>(
				vkGetDeviceProcAddr(device, "vkDestroyCommandPool"));

		retVal.vkResetCommandPool =
			reinterpret_cast<PFN_vkResetCommandPool>(
				vkGetDeviceProcAddr(device, "vkResetCommandPool"));

		retVal.vkAllocateCommandBuffers =
			reinterpret_cast<PFN_vkAllocateCommandBuffers>(
				vkGetDeviceProcAddr(device, "vkAllocateCommandBuffers"));

		retVal.vkBeginCommandBuffer =
			reinterpret_cast<PFN_vkBeginCommandBuffer>(
				vkGetDeviceProcAddr(device, "vkBeginCommandBuffer"));

		retVal.vkEndCommandBuffer =
			reinterpret_cast<PFN_vkEndCommandBuffer>(
				vkGetDeviceProcAddr(device, "vkEndCommandBuffer"));

		retVal.vkCreateSwapchainKHR =
			reinterpret_cast<PFN_vkCreateSwapchainKHR>(
				vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR"));

		retVal.vkDestroySwapchainKHR =
			reinterpret_cast<PFN_vkDestroySwapchainKHR>(
				vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR"));

		retVal.vkGetSwapchainImagesKHR =
			reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(
				vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR"));

		retVal.vkAcquireNextImageKHR =
			reinterpret_cast<PFN_vkAcquireNextImageKHR>(
				vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR"));

		retVal.vkCreateImageView =
			reinterpret_cast<PFN_vkCreateImageView>(
				vkGetDeviceProcAddr(device, "vkCreateImageView"));

		retVal.vkDestroyImageView =
			reinterpret_cast<PFN_vkDestroyImageView>(
				vkGetDeviceProcAddr(device, "vkDestroyImageView"));

		retVal.vkCreateFramebuffer =
			reinterpret_cast<PFN_vkCreateFramebuffer>(
				vkGetDeviceProcAddr(device, "vkCreateFramebuffer"));

		retVal.vkDestroyFramebuffer =
			reinterpret_cast<PFN_vkDestroyFramebuffer>(
				vkGetDeviceProcAddr(device, "vkDestroyFramebuffer"));

		retVal.vkCreateShaderModule =
			reinterpret_cast<PFN_vkCreateShaderModule>(
				vkGetDeviceProcAddr(device, "vkCreateShaderModule"));

		retVal.vkDestroyShaderModule =
			reinterpret_cast<PFN_vkDestroyShaderModule>(
				vkGetDeviceProcAddr(device, "vkDestroyShaderModule"));

		retVal.vkCreateRenderPass =
			reinterpret_cast<PFN_vkCreateRenderPass>(
				vkGetDeviceProcAddr(device, "vkCreateRenderPass"));

		retVal.vkCreateRenderPass2 =
			reinterpret_cast<PFN_vkCreateRenderPass2>(
				vkGetDeviceProcAddr(device, "vkCreateRenderPass2"));

		retVal.vkDestroyRenderPass =
			reinterpret_cast<PFN_vkDestroyRenderPass>(
				vkGetDeviceProcAddr(device, "vkDestroyRenderPass"));

		retVal.vkCreatePipelineLayout =
			reinterpret_cast<PFN_vkCreatePipelineLayout>(
				vkGetDeviceProcAddr(device, "vkCreatePipelineLayout"));

		retVal.vkDestroyPipelineLayout =
			reinterpret_cast<PFN_vkDestroyPipelineLayout>(
				vkGetDeviceProcAddr(device, "vkDestroyPipelineLayout"));

		retVal.vkCreatePipelineCache =
			reinterpret_cast<PFN_vkCreatePipelineCache>(
				vkGetDeviceProcAddr(device, "vkCreatePipelineCache"));

		retVal.vkDestroyPipelineCache =
			reinterpret_cast<PFN_vkDestroyPipelineCache>(
				vkGetDeviceProcAddr(device, "vkDestroyPipelineCache"));

		retVal.vkCreateGraphicsPipelines =
			reinterpret_cast<PFN_vkCreateGraphicsPipelines>(
				vkGetDeviceProcAddr(device, "vkCreateGraphicsPipelines"));

		retVal.vkDestroyPipeline =
			reinterpret_cast<PFN_vkDestroyPipeline>(
				vkGetDeviceProcAddr(device, "vkDestroyPipeline"));

		retVal.vkCreateSemaphore =
			reinterpret_cast<PFN_vkCreateSemaphore>(
				vkGetDeviceProcAddr(device, "vkCreateSemaphore"));

		retVal.vkDestroySemaphore =
			reinterpret_cast<PFN_vkDestroySemaphore>(
				vkGetDeviceProcAddr(device, "vkDestroySemaphore"));

		retVal.vkCreateFence =
			reinterpret_cast<PFN_vkCreateFence>(
				vkGetDeviceProcAddr(device, "vkCreateFence"));

		retVal.vkDestroyFence =
			reinterpret_cast<PFN_vkDestroyFence>(
				vkGetDeviceProcAddr(device, "vkDestroyFence"));

		retVal.vkWaitForFences =
			reinterpret_cast<PFN_vkWaitForFences>(
				vkGetDeviceProcAddr(device, "vkWaitForFences"));

		retVal.vkResetFences =
			reinterpret_cast<PFN_vkResetFences>(
				vkGetDeviceProcAddr(device, "vkResetFences"));

		retVal.vkCreateBuffer =
			reinterpret_cast<PFN_vkCreateBuffer>(
				vkGetDeviceProcAddr(device, "vkCreateBuffer"));

		retVal.vkDestroyBuffer =
			reinterpret_cast<PFN_vkDestroyBuffer>(
				vkGetDeviceProcAddr(device, "vkDestroyBuffer"));

		retVal.vkGetBufferMemoryRequirements =
			reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(
				vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements"));

		retVal.vkBindBufferMemory =
			reinterpret_cast<PFN_vkBindBufferMemory>(
				vkGetDeviceProcAddr(device, "vkBindBufferMemory"));

		retVal.vkAllocateMemory =
			reinterpret_cast<PFN_vkAllocateMemory>(
				vkGetDeviceProcAddr(device, "vkAllocateMemory"));

		retVal.vkFreeMemory =
			reinterpret_cast<PFN_vkFreeMemory>(
				vkGetDeviceProcAddr(device, "vkFreeMemory"));

		retVal.vkMapMemory =
			reinterpret_cast<PFN_vkMapMemory>(
				vkGetDeviceProcAddr(device, "vkMapMemory"));

		retVal.vkUnmapMemory =
			reinterpret_cast<PFN_vkUnmapMemory>(
				vkGetDeviceProcAddr(device, "vkUnmapMemory"));

		retVal.vkFlushMappedMemoryRanges =
			reinterpret_cast<PFN_vkFlushMappedMemoryRanges>(
				vkGetDeviceProcAddr(device, "vkFlushMappedMemoryRanges"));

		// commands

		retVal.vkCmdBeginRenderPass =
			reinterpret_cast<PFN_vkCmdBeginRenderPass>(
				vkGetDeviceProcAddr(device, "vkCmdBeginRenderPass"));

		retVal.vkCmdEndRenderPass =
			reinterpret_cast<PFN_vkCmdEndRenderPass>(
				vkGetDeviceProcAddr(device, "vkCmdEndRenderPass"));

		retVal.vkCmdBindPipeline =
			reinterpret_cast<PFN_vkCmdBindPipeline>(
				vkGetDeviceProcAddr(device, "vkCmdBindPipeline"));

		retVal.vkCmdBindVertexBuffers =
			reinterpret_cast<PFN_vkCmdBindVertexBuffers>(
				vkGetDeviceProcAddr(device, "vkCmdBindVertexBuffers"));

		retVal.vkCmdBindIndexBuffer =
			reinterpret_cast<PFN_vkCmdBindIndexBuffer>(
				vkGetDeviceProcAddr(device, "vkCmdBindIndexBuffer"));
		
		retVal.vkCmdDraw =
			reinterpret_cast<PFN_vkCmdDraw>(
				vkGetDeviceProcAddr(device, "vkCmdDraw"));

		retVal.vkCmdDrawIndexed =
			reinterpret_cast<PFN_vkCmdDrawIndexed>(
				vkGetDeviceProcAddr(device, "vkCmdDrawIndexed"));

		retVal.vkCmdDrawIndexedIndirectCount =
			reinterpret_cast<PFN_vkCmdDrawIndexedIndirectCount>(
				vkGetDeviceProcAddr(device, "vkCmdDrawIndexedIndirectCount"));

		return retVal;
	}

	VulkanLoader &VulkanLoader::getInstance() noexcept
	{
		static VulkanLoader instance;
		return instance;
	}

	void VulkanLoader::__loadGlobalProc(const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) noexcept
	{
		// vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr") 반환 값이 nullptr 나옴
		__globalProc.vkGetInstanceProcAddr = vkGetInstanceProcAddr;

		__globalProc.vkEnumerateInstanceVersion =
			reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
				vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

		__globalProc.vkEnumerateInstanceExtensionProperties =
			reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
				vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties"));

		__globalProc.vkEnumerateInstanceLayerProperties =
			reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
				vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties"));

		__globalProc.vkCreateInstance =
			reinterpret_cast<PFN_vkCreateInstance>(
				vkGetInstanceProcAddr(nullptr, "vkCreateInstance"));
	}
}