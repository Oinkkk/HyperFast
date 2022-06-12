#include "VulkanLoader.h"
#include <exception>
#include <cassert>
#include "../Infrastructure/Environment.h"

namespace VKL
{
	VulkanLoader::~VulkanLoader()
	{
		try
		{
			free();
		}
		catch (...)
		{ }
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
		tf::Taskflow taskflow;

		taskflow.emplace([&]()
		{
			retVal.vkDestroyInstance =
				reinterpret_cast<PFN_vkDestroyInstance>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkDestroyInstance"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateDebugUtilsMessengerEXT =
				reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyDebugUtilsMessengerEXT =
				reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkEnumeratePhysicalDevices =
				reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkEnumeratePhysicalDeviceGroups =
				reinterpret_cast<PFN_vkEnumeratePhysicalDeviceGroups>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroups"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceProperties =
				reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceProperties2 =
				reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceQueueFamilyProperties =
				reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceQueueFamilyProperties2 =
				reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceWin32PresentationSupportKHR =
				reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateWin32SurfaceKHR =
				reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroySurfaceKHR =
				reinterpret_cast<PFN_vkDestroySurfaceKHR>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceSurfaceSupportKHR =
				reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceSurfaceCapabilitiesKHR =
				reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceSurfaceFormatsKHR =
				reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetPhysicalDeviceSurfacePresentModesKHR =
				reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateDevice =
				reinterpret_cast<PFN_vkCreateDevice>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkCreateDevice"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetDeviceProcAddr =
				reinterpret_cast<PFN_vkGetDeviceProcAddr>(
					__globalProc.vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr")); 
		});

		tf::Executor &taskflowExecutor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		taskflowExecutor.run(taskflow).wait();

		return retVal;
	}

	DeviceProcedure VulkanLoader::queryDeviceProcedure(
		const PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr, const VkDevice device) noexcept
	{
		assert(__handle);

		DeviceProcedure retVal;
		tf::Taskflow taskflow;

		taskflow.emplace([&]()
		{
			retVal.vkDestroyDevice =
				reinterpret_cast<PFN_vkDestroyDevice>(
					vkGetDeviceProcAddr(device, "vkDestroyDevice"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDeviceWaitIdle =
				reinterpret_cast<PFN_vkDeviceWaitIdle>(
					vkGetDeviceProcAddr(device, "vkDeviceWaitIdle"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetDeviceQueue =
				reinterpret_cast<PFN_vkGetDeviceQueue>(
					vkGetDeviceProcAddr(device, "vkGetDeviceQueue"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkQueueWaitIdle =
				reinterpret_cast<PFN_vkQueueWaitIdle>(
					vkGetDeviceProcAddr(device, "vkQueueWaitIdle"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkQueueSubmit =
				reinterpret_cast<PFN_vkQueueSubmit>(
					vkGetDeviceProcAddr(device, "vkQueueSubmit"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkQueueSubmit2 =
				reinterpret_cast<PFN_vkQueueSubmit2>(
					vkGetDeviceProcAddr(device, "vkQueueSubmit2"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkQueuePresentKHR =
				reinterpret_cast<PFN_vkQueuePresentKHR>(
					vkGetDeviceProcAddr(device, "vkQueuePresentKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateCommandPool =
				reinterpret_cast<PFN_vkCreateCommandPool>(
					vkGetDeviceProcAddr(device, "vkCreateCommandPool"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyCommandPool =
				reinterpret_cast<PFN_vkDestroyCommandPool>(
					vkGetDeviceProcAddr(device, "vkDestroyCommandPool"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkResetCommandPool =
				reinterpret_cast<PFN_vkResetCommandPool>(
					vkGetDeviceProcAddr(device, "vkResetCommandPool"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkAllocateCommandBuffers =
				reinterpret_cast<PFN_vkAllocateCommandBuffers>(
					vkGetDeviceProcAddr(device, "vkAllocateCommandBuffers"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkBeginCommandBuffer =
				reinterpret_cast<PFN_vkBeginCommandBuffer>(
					vkGetDeviceProcAddr(device, "vkBeginCommandBuffer"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkEndCommandBuffer =
				reinterpret_cast<PFN_vkEndCommandBuffer>(
					vkGetDeviceProcAddr(device, "vkEndCommandBuffer"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateSwapchainKHR =
				reinterpret_cast<PFN_vkCreateSwapchainKHR>(
					vkGetDeviceProcAddr(device, "vkCreateSwapchainKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroySwapchainKHR =
				reinterpret_cast<PFN_vkDestroySwapchainKHR>(
					vkGetDeviceProcAddr(device, "vkDestroySwapchainKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetSwapchainImagesKHR =
				reinterpret_cast<PFN_vkGetSwapchainImagesKHR>(
					vkGetDeviceProcAddr(device, "vkGetSwapchainImagesKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkAcquireNextImageKHR =
				reinterpret_cast<PFN_vkAcquireNextImageKHR>(
					vkGetDeviceProcAddr(device, "vkAcquireNextImageKHR"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateImageView =
				reinterpret_cast<PFN_vkCreateImageView>(
					vkGetDeviceProcAddr(device, "vkCreateImageView"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyImageView =
				reinterpret_cast<PFN_vkDestroyImageView>(
					vkGetDeviceProcAddr(device, "vkDestroyImageView"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateFramebuffer =
				reinterpret_cast<PFN_vkCreateFramebuffer>(
					vkGetDeviceProcAddr(device, "vkCreateFramebuffer"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyFramebuffer =
				reinterpret_cast<PFN_vkDestroyFramebuffer>(
					vkGetDeviceProcAddr(device, "vkDestroyFramebuffer"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateShaderModule =
				reinterpret_cast<PFN_vkCreateShaderModule>(
					vkGetDeviceProcAddr(device, "vkCreateShaderModule"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyShaderModule =
				reinterpret_cast<PFN_vkDestroyShaderModule>(
					vkGetDeviceProcAddr(device, "vkDestroyShaderModule"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateRenderPass =
				reinterpret_cast<PFN_vkCreateRenderPass>(
					vkGetDeviceProcAddr(device, "vkCreateRenderPass"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateRenderPass2 =
				reinterpret_cast<PFN_vkCreateRenderPass2>(
					vkGetDeviceProcAddr(device, "vkCreateRenderPass2"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyRenderPass =
				reinterpret_cast<PFN_vkDestroyRenderPass>(
					vkGetDeviceProcAddr(device, "vkDestroyRenderPass"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreatePipelineLayout =
				reinterpret_cast<PFN_vkCreatePipelineLayout>(
					vkGetDeviceProcAddr(device, "vkCreatePipelineLayout"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyPipelineLayout =
				reinterpret_cast<PFN_vkDestroyPipelineLayout>(
					vkGetDeviceProcAddr(device, "vkDestroyPipelineLayout"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreatePipelineCache =
				reinterpret_cast<PFN_vkCreatePipelineCache>(
					vkGetDeviceProcAddr(device, "vkCreatePipelineCache"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyPipelineCache =
				reinterpret_cast<PFN_vkDestroyPipelineCache>(
					vkGetDeviceProcAddr(device, "vkDestroyPipelineCache"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateGraphicsPipelines =
				reinterpret_cast<PFN_vkCreateGraphicsPipelines>(
					vkGetDeviceProcAddr(device, "vkCreateGraphicsPipelines"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyPipeline =
				reinterpret_cast<PFN_vkDestroyPipeline>(
					vkGetDeviceProcAddr(device, "vkDestroyPipeline"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCmdBeginRenderPass =
				reinterpret_cast<PFN_vkCmdBeginRenderPass>(
					vkGetDeviceProcAddr(device, "vkCmdBeginRenderPass"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCmdEndRenderPass =
				reinterpret_cast<PFN_vkCmdEndRenderPass>(
					vkGetDeviceProcAddr(device, "vkCmdEndRenderPass"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCmdBindPipeline =
				reinterpret_cast<PFN_vkCmdBindPipeline>(
					vkGetDeviceProcAddr(device, "vkCmdBindPipeline"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCmdDraw =
				reinterpret_cast<PFN_vkCmdDraw>(
					vkGetDeviceProcAddr(device, "vkCmdDraw"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateSemaphore =
				reinterpret_cast<PFN_vkCreateSemaphore>(
					vkGetDeviceProcAddr(device, "vkCreateSemaphore"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroySemaphore =
				reinterpret_cast<PFN_vkDestroySemaphore>(
					vkGetDeviceProcAddr(device, "vkDestroySemaphore"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateFence =
				reinterpret_cast<PFN_vkCreateFence>(
					vkGetDeviceProcAddr(device, "vkCreateFence"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyFence =
				reinterpret_cast<PFN_vkDestroyFence>(
					vkGetDeviceProcAddr(device, "vkDestroyFence"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkWaitForFences =
				reinterpret_cast<PFN_vkWaitForFences>(
					vkGetDeviceProcAddr(device, "vkWaitForFences"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkResetFences =
				reinterpret_cast<PFN_vkResetFences>(
					vkGetDeviceProcAddr(device, "vkResetFences"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkCreateBuffer =
				reinterpret_cast<PFN_vkCreateBuffer>(
					vkGetDeviceProcAddr(device, "vkCreateBuffer"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkDestroyBuffer =
				reinterpret_cast<PFN_vkDestroyBuffer>(
					vkGetDeviceProcAddr(device, "vkDestroyBuffer"));
		});

		taskflow.emplace([&]()
		{
			retVal.vkGetBufferMemoryRequirements =
				reinterpret_cast<PFN_vkGetBufferMemoryRequirements>(
					vkGetDeviceProcAddr(device, "vkGetBufferMemoryRequirements"));
		});

		tf::Executor &taskflowExecutor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		taskflowExecutor.run(taskflow).wait();

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

		tf::Taskflow taskflow;

		taskflow.emplace([this, vkGetInstanceProcAddr]()
		{
			__globalProc.vkEnumerateInstanceVersion =
				reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
					vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));
		});

		taskflow.emplace([this, vkGetInstanceProcAddr]()
		{
			__globalProc.vkEnumerateInstanceExtensionProperties =
				reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
					vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties"));
		});

		taskflow.emplace([this, vkGetInstanceProcAddr]()
		{
			__globalProc.vkEnumerateInstanceLayerProperties =
				reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
					vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties"));
		});

		taskflow.emplace([this, vkGetInstanceProcAddr]()
		{
			__globalProc.vkCreateInstance =
				reinterpret_cast<PFN_vkCreateInstance>(
					vkGetInstanceProcAddr(nullptr, "vkCreateInstance"));
		});

		tf::Executor &taskflowExecutor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		taskflowExecutor.run(taskflow).wait();
	}
}