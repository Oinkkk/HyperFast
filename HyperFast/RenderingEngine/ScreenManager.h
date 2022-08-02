#pragma once

#include "../Infrastructure/TemporalDeleter.h"
#include "../Window/Window.h"
#include "../Vulkan/Queue.h"
#include "../Vulkan/Fence.h"
#include "../Vulkan/Surface.h"
#include "../Vulkan/Swapchain.h"
#include "../Vulkan/ImageView.h"
#include "../Vulkan/RenderPass.h"
#include "../Vulkan/Framebuffer.h"
#include "LifeCycle.h"
#include "CommandSubmitter.h"
#include "Drawcall.h"
#include "PipelineFactory.h"

namespace HyperFast
{
	class ScreenManager final : public Infra::Unique
	{
	public:
		class ScreenImpl final : public Infra::Unique
		{
		public:
			ScreenImpl(
				Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
				const uint32_t queueFamilyIndex, Vulkan::Device &device,
				Vulkan::Queue &queue, LifeCycle &lifeCycle,
				CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter,
				Win::Window &window);

			~ScreenImpl() noexcept;

			void setDrawcall(Drawcall *const pDrawcall) noexcept;

		private:
			class PerImageCommandBufferResource
			{
			public:
				std::unique_ptr<CommandBufferManager> primaryManager;
				std::unordered_map<size_t, std::unique_ptr<CommandBufferManager>> secondaryManagerMap;
				std::vector<VkCommandBuffer> secondaryCommandBufferHandles;
			};

			// External params

			Vulkan::Instance &__instance;
			Vulkan::PhysicalDevice &__physicalDevice;
			Vulkan::Device &__device;

			const uint32_t __queueFamilyIndex;
			Vulkan::Queue &__queue;

			LifeCycle &__lifeCycle;
			CommandSubmitter &__commandSubmitter;
			Infra::TemporalDeleter &__resourceDeleter;

			Win::Window &__window;

			Drawcall *__pDrawcall{};


			// Internal params

			Vulkan::Surface *__pSurface{};
			VkSurfaceCapabilitiesKHR __surfaceCapabilities{};
			std::vector<VkSurfaceFormatKHR> __supportedSurfaceFormats;
			std::vector<VkPresentModeKHR> __supportedSurfacePresentModes;

			Vulkan::Swapchain *__pSwapchain{};
			VkFormat __swapchainFormat{};
			VkExtent2D __swapchainExtent{};
			std::vector<VkImage> __swapChainImages;
			std::vector<Vulkan::ImageView *> __swapChainImageViews;

			Vulkan::RenderPass *__pRenderPass;
			Vulkan::Framebuffer *__pFramebuffer;

			PipelineFactory::BuildParam __pipelineBuildParam;
			std::unique_ptr<PipelineFactory> __pPipelineFactory;

			std::unordered_map<size_t, std::unique_ptr<PerImageCommandBufferResource>> __perImageCommandBufferResources;
			VkCommandBufferInheritanceInfo __secondaryCommandBufferInheritanceInfo{};
			VkCommandBufferBeginInfo __secondaryCommandBufferBeginInfo{};

			// Flags

			bool __swapchainDependencDirty{};
			bool __pipelineDependencyDirty{};
			bool __commandBufferDirty{};
			std::unordered_set<size_t> __drawcallSegmentDirties;

			bool __needToRender{};
			bool __needToPresent{};

			bool __destroyed{};


			// Event listeners

			std::shared_ptr<Infra::EventListener<Win::Window &, Win::Window::ResizingType>> __pWindowResizeEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pWindowDrawEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pWindowDestroyEventListener;

			std::shared_ptr<Infra::EventListener<Drawcall &, size_t>> __pDrawcallMeshBufferChangeEventListener;
			std::shared_ptr<Infra::EventListener<Drawcall &, size_t>> __pDrawcallIndirectBufferUpdateEventListener;
			std::shared_ptr<Infra::EventListener<Drawcall &, size_t>> __pDrawcallIndirectBufferCreateEventListener;

			std::shared_ptr<Infra::EventListener<>> __pScreenUpdateListener;
			std::shared_ptr<Infra::EventListener<>> __pRenderListener;
			std::shared_ptr<Infra::EventListener<>> __pPresentListener;


			// functions

			[[nodiscard]]
			bool __isUpdatable() const noexcept;

			[[nodiscard]]
			bool __isRenderable() const noexcept;

			[[nodiscard]]
			bool __isPresentable() const noexcept;

			void __update();
			void __render() noexcept;
			void __present() noexcept;

			void __destroy() noexcept;

			void __initListeners() noexcept;
			void __registerListeners() noexcept;
			void __createSurface();
			void __createPipelineFactory() noexcept;
			constexpr void __initSecondaryCommandBufferBeginInfos() noexcept;

			void __resetSwapchainDependencies() noexcept;

			void __updateSwapchainDependencies();
			void __updatePipelineDependencies();
			void __updateCommandBuffers();

			void __checkSurfaceSupport() const;
			void __querySurfaceCapabilities() noexcept;
			void __querySupportedSurfaceFormats() noexcept;
			void __querySupportedSurfacePresentModes() noexcept;
			void __populateSecondaryCommandBufferInheritanceInfo() noexcept;
			void __dirtyAllDrawcallSegments() noexcept;

			void __createSwapchain();
			void __createSwapchainImageViews();
			void __createRenderPass();
			void __createFramebuffer();

			void __buildPipelines(tf::Subflow &subflow);
			void __updateSecondaryCommandBuffers(
				PerImageCommandBufferResource &commandBufferResource, tf::Subflow &subflow) noexcept;

			void __updatePrimaryCommandBuffer(
				PerImageCommandBufferResource &commandBufferResource, const size_t imageIdx) noexcept;

			void __onWindowResize(
				Win::Window &window, const Win::Window::ResizingType resizingType) noexcept;

			void __onDrawcallMeshBufferChange(Drawcall &drawcall, const size_t segmentIdx) noexcept;
			void __onDrawcallIndirectBufferUpdate(Drawcall &drawcall, const size_t segmentIdx) noexcept;
			void __onDrawcallIndirectBufferCreate(Drawcall &drawcall, const size_t segmentIdx) noexcept;

			void __onWindowDraw(Win::Window &window) noexcept;
			void __onWindowDestroy(Win::Window &window) noexcept;

			void __onScreenUpdate();
			void __onRender() noexcept;
			void __onPresent() noexcept;

			[[nodiscard]]
			PerImageCommandBufferResource &__getPerImageCommandBufferResource(const size_t imageIdx) noexcept;

			[[nodiscard]]
			Vulkan::CommandBuffer &__nextSecondaryCommandBuffer(
				PerImageCommandBufferResource &resource, const size_t segmentIdx);

			void __updateSecondaryCommandBufferHandles(
				PerImageCommandBufferResource &commandBufferResource) noexcept;

			[[nodiscard]]
			Vulkan::CommandBuffer &__nextPrimaryCommandBuffer(PerImageCommandBufferResource &resource);
		};

		ScreenManager(
			Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
			const uint32_t graphicsQueueFamilyIndex, Vulkan::Device &device, Vulkan::Queue &queue,
			LifeCycle &lifeCycle, CommandSubmitter &commandSubmitter,
			Infra::TemporalDeleter &resourceDeleter) noexcept;

		~ScreenManager() noexcept = default;

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

	constexpr void ScreenManager::ScreenImpl::__initSecondaryCommandBufferBeginInfos() noexcept
	{
		__secondaryCommandBufferInheritanceInfo.sType =
			VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

		__secondaryCommandBufferInheritanceInfo.subpass = 0U;

		__secondaryCommandBufferBeginInfo.sType =
			VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		__secondaryCommandBufferBeginInfo.flags =
			VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;

		__secondaryCommandBufferBeginInfo.pInheritanceInfo = &__secondaryCommandBufferInheritanceInfo;
	}
}