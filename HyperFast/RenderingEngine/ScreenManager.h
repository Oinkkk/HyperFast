#pragma once

#include "../Window/Window.h"
#include "../Infrastructure/Logger.h"
#include "../Vulkan/Queue.h"
#include "../Vulkan/Fence.h"
#include "../Vulkan/Surface.h"
#include "ScreenResource.h"
#include "TimelineSemaphore.h"

namespace HyperFast
{
	class RenderingEngine;

	class ScreenManager final : public Infra::Unique
	{
	public:
		class ScreenImpl final : public Infra::Unique
		{
		public:
			ScreenImpl(
				RenderingEngine &renderingEngine,
				Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
				const uint32_t queueFamilyIndex, Vulkan::Device &device,
				Vulkan::Queue &queue, Win::Window &window);

			~ScreenImpl() noexcept;

			void setDrawcall(Drawcall *const pDrawcall) noexcept;

		private:
			RenderingEngine &__renderingEngine;
			Vulkan::Instance &__instance;
			Vulkan::PhysicalDevice &__physicalDevice;
			Vulkan::Device &__device;

			const uint32_t __queueFamilyIndex;
			Vulkan::Queue &__queue;

			std::unique_ptr<ScreenResource> __resourceChain[3];
			size_t __resourceCursor{};
			bool __resourceChainInit{};

			Win::Window &__window;
			std::shared_ptr<Infra::EventListener<Win::Window &, Win::Window::ResizingType>> __pResizeEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pDrawEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pDestroyEventListener;

			std::shared_ptr<Infra::EventListener<Drawcall &>> __pAttribFlagsUpdateEventListener;
			std::shared_ptr<Infra::EventListener<Drawcall &>> __pIndirectBufferUpdateListener;
			std::shared_ptr<Infra::EventListener<Drawcall &>> __pIndirectBufferCreateListener;
			std::shared_ptr<Infra::EventListener<>> __pScreenUpdateListener;
			std::shared_ptr<Infra::EventListener<>> __pRenderListener;
			std::shared_ptr<Infra::EventListener<>> __pPresentListener;

			std::unique_ptr<Vulkan::Surface> __pSurface;
			VkSurfaceCapabilitiesKHR __surfaceCapabilities{};
			std::vector<VkSurfaceFormatKHR> __supportedSurfaceFormats;
			std::vector<VkPresentModeKHR> __supportedSurfacePresentModes;

			std::unique_ptr<Vulkan::Swapchain> __pSwapchain;
			std::unique_ptr<Vulkan::Swapchain> __pOldSwapchain;

			ScreenResource::ExternalParam __resourceParam;

			VkSemaphoreSubmitInfo __submitWaitInfo{};
			VkCommandBufferSubmitInfo __submitCommandBufferInfo{};
			VkSemaphoreSubmitInfo __submitSignalInfos[2]{};

			std::vector<std::unique_ptr<Vulkan::Semaphore>> __imageAcquireSemaphores;
			std::vector<std::unique_ptr<Vulkan::Semaphore>> __renderCompletionBinarySemaphores;
			std::vector<std::unique_ptr<TimelineSemaphore>> __renderCompletionTimelineSemaphores;
			std::vector<uint64_t> __renderCompletionSemaphoreValues;

			size_t __frameCursor{};
			bool __imageAcquired{};
			uint32_t __imageIdx{};

			bool __needToUpdateSurfaceDependencies{ true };
			bool __needToUpdatePipelineDependencies{};
			bool __needToUpdateMainCommands{};
			bool __needToUpdateResource{};
			bool __needToAdvanceResource{};

			bool __needToRender{};
			bool __needToPresent{};

			bool __destroyed{};

			void __update();
			void __render() noexcept;
			void __present() noexcept;

			void __destroy() noexcept;

			void __initListeners() noexcept;
			void __registerListeners() noexcept;
			void __createResourceChain() noexcept;
			void __createSurface();
			constexpr void __initSubmitInfo() noexcept;

			void __updateSurfaceDependencies();
			void __updatePipelineDependencies();
			void __updateMainCommands();
			void __updateResource();
			void __advanceResource() noexcept;

			void __checkSurfaceSupport() const;
			void __querySurfaceCapabilities() noexcept;
			void __querySupportedSurfaceFormats() noexcept;
			void __querySupportedSurfacePresentModes() noexcept;
			void __createSwapchain();
			void __createRenderSemaphores(const size_t imageIdx);

			constexpr void __advanceFrameCursor() noexcept;

			[[nodiscard]]
			bool __isValid() const noexcept;

			[[nodiscard]]
			bool __isRenderable() const noexcept;

			[[nodiscard]]
			ScreenResource &__getCurrentResource() noexcept;

			[[nodiscard]]
			ScreenResource &__getNextResource() noexcept;

			[[nodiscard]]
			Vulkan::Semaphore &__getCurrentImageAcquireSemaphore() noexcept;

			[[nodiscard]]
			Vulkan::CommandBuffer &__getCurrentRenderCommandBuffer() noexcept;

			[[nodiscard]]
			Vulkan::Semaphore &__getCurrentRenderCompletionBinarySemaphore() noexcept;

			[[nodiscard]]
			TimelineSemaphore &__getCurrentRenderCompletionTimelineSemaphore() noexcept;

			[[nodiscard]]
			bool __acquireNextSwapchainImageIdx(Vulkan::Semaphore &semaphore) noexcept;

			void __onScreenUpdate();
			void __onRender() noexcept;
			void __onPresent() noexcept;
		};

		ScreenManager(
			RenderingEngine &renderingEngine,
			Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
			const uint32_t graphicsQueueFamilyIndex,
			Vulkan::Device &device, Vulkan::Queue &queue) noexcept;

		~ScreenManager() noexcept = default;

		[[nodiscard]]
		std::unique_ptr<ScreenImpl> create(Win::Window &window) noexcept;

	private:
		RenderingEngine &__renderingEngine;
		Vulkan::Instance &__instance;
		Vulkan::PhysicalDevice &__physicalDevice;
		Vulkan::Device &__device;

		const uint32_t __queueFamilyIndex;
		Vulkan::Queue &__queue;
	};

	constexpr void ScreenManager::ScreenImpl::__initSubmitInfo() noexcept
	{
		__submitWaitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		__submitWaitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		__submitCommandBufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;

		__submitSignalInfos[0].sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		__submitSignalInfos[0].stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		__submitSignalInfos[1].sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		__submitSignalInfos[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	}

	constexpr void ScreenManager::ScreenImpl::__advanceFrameCursor() noexcept
	{
		const size_t numSwapchainImages{ __resourceParam.swapChainImages.size() };
		__frameCursor = ((__frameCursor + 1ULL) % numSwapchainImages);
	}
}