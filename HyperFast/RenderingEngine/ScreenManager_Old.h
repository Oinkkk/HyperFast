#pragma once

#include "../Window/Window.h"
#include "../Infrastructure/Logger.h"
#include "../Vulkan/Queue.h"
#include "../Vulkan/Fence.h"
#include "../Vulkan/Surface.h"
#include "ScreenResource_Old.h"
#include "LifeCycle.h"
#include "CommandSubmitter.h"

namespace HyperFast
{
	class ScreenManager_Old final : public Infra::Unique
	{
	public:
		class ScreenImpl final : public Infra::Unique
		{
		public:
			ScreenImpl(
				Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
				const uint32_t queueFamilyIndex, Vulkan::Device &device,
				Vulkan::Queue &queue, Win::Window &window,
				LifeCycle &lifeCycle, CommandSubmitter &commandSubmitter,
				Infra::TemporalDeleter &resourceDeleter);

			~ScreenImpl() noexcept;

			void setDrawcall(Drawcall *const pDrawcall) noexcept;

		private:
			Vulkan::Instance &__instance;
			Vulkan::PhysicalDevice &__physicalDevice;
			Vulkan::Device &__device;

			const uint32_t __queueFamilyIndex;
			Vulkan::Queue &__queue;

			LifeCycle &__lifeCycle;
			CommandSubmitter &__commandSubmitter;
			Infra::TemporalDeleter &__resourceDeleter;

			std::unique_ptr<ScreenResource> __resourceChain[3];
			size_t __resourceCursor{};
			bool __resourceChainInit{};

			Win::Window &__window;
			std::shared_ptr<Infra::EventListener<Win::Window &, Win::Window::ResizingType>> __pWindowResizeEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pWindowDrawEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pWindowDestroyEventListener;

			std::shared_ptr<Infra::EventListener<Drawcall &, size_t>> __pDrawcallMeshBufferChangeEventListener;
			std::shared_ptr<Infra::EventListener<Drawcall &, size_t>> __pDrawcallIndirectBufferUpdateEventListener;
			std::shared_ptr<Infra::EventListener<Drawcall &, size_t>> __pDrawcallIndirectBufferCreateEventListener;

			std::shared_ptr<Infra::EventListener<>> __pScreenUpdateListener;
			std::shared_ptr<Infra::EventListener<>> __pRenderListener;
			std::shared_ptr<Infra::EventListener<>> __pPresentListener;

			std::unique_ptr<Vulkan::Surface> __pSurface;
			VkSurfaceCapabilitiesKHR __surfaceCapabilities{};
			std::vector<VkSurfaceFormatKHR> __supportedSurfaceFormats;
			std::vector<VkPresentModeKHR> __supportedSurfacePresentModes;

			std::unique_ptr<Vulkan::Swapchain> __pSwapchain;
			std::unique_ptr<Vulkan::Swapchain> __pOldSwapchain;
			ScreenResource::SwapchainParam __swapchainParam;
			Drawcall *__pDrawcall{};

			VkSemaphoreSubmitInfo __submitWaitInfo{};
			VkCommandBufferSubmitInfo __submitCommandBufferInfo{};
			VkSemaphoreSubmitInfo __submitSignalInfos[2]{};

			SemaphoreDependencyManager __submitDependencyManager;
			std::shared_ptr<SemaphoreDependency> __pCurrentSubmitDependency;

			std::vector<std::unique_ptr<Vulkan::Semaphore>> __imageAcquireSemaphores;
			std::vector<std::unique_ptr<Vulkan::Semaphore>> __renderCompletionBinarySemaphores;
			std::vector<std::unique_ptr<Vulkan::Semaphore>> __renderCompletionTimelineSemaphores;
			std::vector<uint64_t> __renderCompletionSemaphoreValues;

			size_t __frameCursor{};
			bool __imageAcquired{};
			uint32_t __imageIdx{};

			bool __swapchainDependencDirty{ true };
			bool __pipelineDependencyDirty{};
			bool __commandBufferDirty{};
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

			void __updateSwapchainDependencies();
			void __updatePipelineDependencies();
			void __updateCommandBuffers();
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
			bool __isUpdatable() const noexcept;

			[[nodiscard]]
			bool __isRenderable() const noexcept;

			[[nodiscard]]
			ScreenResource &__getCurrentResource() noexcept;

			[[nodiscard]]
			ScreenResource &__getNextResource() noexcept;

			[[nodiscard]]
			Vulkan::Semaphore &__getCurrentImageAcquireSemaphore() noexcept;

			[[nodiscard]]
			Vulkan::CommandBuffer &__getCurrentPrimaryCommandBuffer() noexcept;

			[[nodiscard]]
			Vulkan::Semaphore &__getCurrentRenderCompletionBinarySemaphore() noexcept;

			[[nodiscard]]
			Vulkan::Semaphore &__getCurrentRenderCompletionTimelineSemaphore() noexcept;

			[[nodiscard]]
			uint64_t &__getCurrentRenderCompletionSemaphoreValue() noexcept;

			[[nodiscard]]
			bool __acquireNextSwapchainImageIdx(Vulkan::Semaphore &semaphore) noexcept;

			void __onWindowResize(
				Win::Window &window, const Win::Window::ResizingType resizingType) noexcept;

			void __onDrawcallMeshBufferChange(Drawcall &drawcall, const size_t segmentIndex) noexcept;
			void __onDrawcallIndirectBufferUpdate(Drawcall &drawcall, const size_t segmentIndex) noexcept;
			void __onDrawcallIndirectBufferCreate(Drawcall &drawcall, const size_t segmentIndex) noexcept;

			void __onWindowDraw(Win::Window &window) noexcept;
			void __onWindowDestroy(Win::Window &window) noexcept;

			void __onScreenUpdate();
			void __onRender() noexcept;
			void __onPresent() noexcept;
		};

		ScreenManager_Old(
			Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
			const uint32_t graphicsQueueFamilyIndex, Vulkan::Device &device, Vulkan::Queue &queue,
			LifeCycle &lifeCycle, CommandSubmitter &commandSubmitter,
			Infra::TemporalDeleter &resourceDeleter) noexcept;

		~ScreenManager_Old() noexcept = default;

		[[nodiscard]]
		std::unique_ptr<ScreenImpl> create(Win::Window &window) noexcept;

	private:
		Vulkan::Instance &__instance;
		Vulkan::PhysicalDevice &__physicalDevice;
		Vulkan::Device &__device;

		const uint32_t __queueFamilyIndex;
		Vulkan::Queue &__queue;

		LifeCycle &__lifeCycle;
		CommandSubmitter &__commandSubmitter;
		Infra::TemporalDeleter &__resourceDeleter;
	};

	constexpr void ScreenManager_Old::ScreenImpl::__initSubmitInfo() noexcept
	{
		__submitWaitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		__submitWaitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		__submitCommandBufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;

		__submitSignalInfos[0].sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		__submitSignalInfos[0].stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		__submitSignalInfos[1].sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		__submitSignalInfos[1].stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
	}

	constexpr void ScreenManager_Old::ScreenImpl::__advanceFrameCursor() noexcept
	{
		const size_t numSwapchainImages{ __swapchainParam.swapChainImages.size() };
		__frameCursor = ((__frameCursor + 1ULL) % numSwapchainImages);
	}
}