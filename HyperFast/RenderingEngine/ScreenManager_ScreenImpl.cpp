#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenImpl::ScreenImpl(
		const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
		const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc, const VkQueue graphicsQueue,
		Win::Window &window, Infra::Logger &logger) :
		__instance{ instance }, __instanceProc{ instanceProc },
		__physicalDevice{ physicalDevice }, __graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __deviceProc{ deviceProc }, __graphicsQueue{ graphicsQueue },
		__window{ window }, __logger{ logger }
	{
		tf::Taskflow taskflow;
		tf::Task t1
		{
			taskflow.emplace([this] (tf::Subflow &subflow)
			{
				__createSurface();
				__createResourceBundle();
				__pResourceBundle->update(subflow);
			})
		};

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__available = executor.run(std::move(taskflow));
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__available.wait();
		__waitDeviceIdle();

		__destroyResourceBundle();
		__destroySurface();
	}

	bool ScreenManager::ScreenImpl::draw()
	{
		__available.wait();

		const VkSemaphore presentCompleteSemaphore{ __pResourceBundle->getPresentCompleteSemaphore() };

		uint32_t imageIdx{};
		VkResult acquirementResult{ __pResourceBundle->acquireNextImage(presentCompleteSemaphore, imageIdx) };

		if (acquirementResult == VkResult::VK_NOT_READY)
			return false;

		if ((acquirementResult == VkResult::VK_SUBOPTIMAL_KHR) ||
			(acquirementResult == VkResult::VK_ERROR_OUT_OF_DATE_KHR))
		{
			__updateSurfaceDependencies();
			return false;
		}

		if (acquirementResult != VkResult::VK_SUCCESS)
			throw std::exception{ "Error occurred while drawing" };

		__pResourceBundle->nextFrame();

		const VkCommandBuffer mainCommandBuffer{ __pResourceBundle->getMainCommandBuffer(imageIdx) };
		const VkSemaphore renderCompleteSemaphore{ __pResourceBundle->getRenderCompleteSemaphore(imageIdx) };
		const VkFence renderCompleteFence{ __pResourceBundle->getRenderCompleteFence(imageIdx) };

		const VkSemaphoreSubmitInfo waitInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = presentCompleteSemaphore,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		const VkCommandBufferSubmitInfo commandBufferInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = mainCommandBuffer
		};

		const VkSemaphoreSubmitInfo signalInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = renderCompleteSemaphore,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		const VkSubmitInfo2 submitInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 1U,
			.pWaitSemaphoreInfos = &waitInfo,
			.commandBufferInfoCount = 1U,
			.pCommandBufferInfos = &commandBufferInfo,
			.signalSemaphoreInfoCount = 1U,
			.pSignalSemaphoreInfos = &signalInfo
		};

		__deviceProc.vkWaitForFences(__device, 1U, &renderCompleteFence, VK_TRUE, __maxTime);
		__deviceProc.vkResetFences(__device, 1U, &renderCompleteFence);
		__deviceProc.vkQueueSubmit2(__graphicsQueue, 1U, &submitInfo, renderCompleteFence);

		const VkSwapchainKHR swapchain{ __pResourceBundle->getSwapchain() };
		const VkPresentInfoKHR presentInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1U,
			.pWaitSemaphores = &renderCompleteSemaphore,
			.swapchainCount = 1U,
			.pSwapchains = &swapchain,
			.pImageIndices = &imageIdx
		};

		const VkResult presentResult{ __deviceProc.vkQueuePresentKHR(__graphicsQueue, &presentInfo) };
		return (presentResult == VkResult::VK_SUCCESS);
	}

	void ScreenManager::ScreenImpl::__createSurface()
	{
		Win::WindowClass &windowClass{ __window.getClass() };

		const VkWin32SurfaceCreateInfoKHR createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = windowClass.getHInstance(),
			.hwnd = __window.getHandle()
		};

		__instanceProc.vkCreateWin32SurfaceKHR(__instance, &createInfo, nullptr, &__surface);
		if (!__surface)
			throw std::exception{ "Cannot create a surface." };
	}

	void ScreenManager::ScreenImpl::__destroySurface() noexcept
	{
		__instanceProc.vkDestroySurfaceKHR(__instance, __surface, nullptr);
	}

	void ScreenManager::ScreenImpl::__updateSurfaceDependencies()
	{
		tf::Taskflow taskflow;
		taskflow.emplace([this](tf::Subflow &subflow)
		{
			__waitDeviceIdle();
			__pResourceBundle->update(subflow);
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__available = executor.run(std::move(taskflow));
	}

	void ScreenManager::ScreenImpl::__createResourceBundle() noexcept
	{
		__pResourceBundle = std::make_unique<ResourceBundle>(
			__instance, __instanceProc, __window, __surface,
			__physicalDevice, __graphicsQueueFamilyIndex, __device, __deviceProc);
	}

	void ScreenManager::ScreenImpl::__destroyResourceBundle() noexcept
	{
		__pResourceBundle = nullptr;
	}

	void ScreenManager::ScreenImpl::__waitDeviceIdle() const noexcept
	{
		__deviceProc.vkDeviceWaitIdle(__device);
	}
}