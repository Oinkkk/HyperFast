#pragma once

#include <vulkan/vulkan.h>

namespace VKL
{
	class GlobalProcedure
	{
	public:
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr{};
		PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion{};
		PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties{};
		PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties{};
		PFN_vkCreateInstance vkCreateInstance{};
	};

	class InstanceProcedure
	{
	public:
		PFN_vkDestroyInstance vkDestroyInstance{};
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{};
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT{};
		PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices{};
		PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups{};
		PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties{};
		PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2{};
		PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties{};
		PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2{};
		PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR{};
		PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR{};
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR{};
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR{};
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR{};
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR{};
		PFN_vkCreateDevice vkCreateDevice{};
		PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr{};
	};

	class DeviceProcedure
	{
	public:
		PFN_vkDestroyDevice vkDestroyDevice{};
		PFN_vkDeviceWaitIdle vkDeviceWaitIdle{};
		PFN_vkGetDeviceQueue vkGetDeviceQueue{};
		PFN_vkQueueWaitIdle vkQueueWaitIdle{};
		PFN_vkCreateCommandPool vkCreateCommandPool{};
		PFN_vkDestroyCommandPool vkDestroyCommandPool{};
		PFN_vkResetCommandPool vkResetCommandPool{};
		PFN_vkAllocateCommandBuffers vkAllocateCommandBuffers{};
		PFN_vkBeginCommandBuffer vkBeginCommandBuffer{};
		PFN_vkEndCommandBuffer vkEndCommandBuffer{};
		PFN_vkCreateSwapchainKHR vkCreateSwapchainKHR{};
		PFN_vkDestroySwapchainKHR vkDestroySwapchainKHR{};
		PFN_vkGetSwapchainImagesKHR vkGetSwapchainImagesKHR{};
		PFN_vkAcquireNextImageKHR vkAcquireNextImageKHR{};
		PFN_vkCreateImageView vkCreateImageView{};
		PFN_vkDestroyImageView vkDestroyImageView{};
		PFN_vkCreateFramebuffer vkCreateFramebuffer{};
		PFN_vkDestroyFramebuffer vkDestroyFramebuffer{};
		PFN_vkCreateShaderModule vkCreateShaderModule{};
		PFN_vkDestroyShaderModule vkDestroyShaderModule{};
		PFN_vkCreateRenderPass vkCreateRenderPass{};
		PFN_vkDestroyRenderPass vkDestroyRenderPass{};
		PFN_vkCreatePipelineLayout vkCreatePipelineLayout{};
		PFN_vkDestroyPipelineLayout vkDestroyPipelineLayout{};
		PFN_vkCreatePipelineCache vkCreatePipelineCache{};
		PFN_vkDestroyPipelineCache vkDestroyPipelineCache{};
		PFN_vkCreateGraphicsPipelines vkCreateGraphicsPipelines{};
		PFN_vkDestroyPipeline vkDestroyPipeline{};
		PFN_vkCreateSemaphore vkCreateSemaphore{};
		PFN_vkDestroySemaphore vkDestroySemaphore{};
		PFN_vkCreateFence vkCreateFence{};
		PFN_vkDestroyFence vkDestroyFence{};

		PFN_vkCmdBeginRenderPass vkCmdBeginRenderPass{};
		PFN_vkCmdEndRenderPass vkCmdEndRenderPass{};
		PFN_vkCmdBindPipeline vkCmdBindPipeline{};
		PFN_vkCmdDraw vkCmdDraw{};
	};
}