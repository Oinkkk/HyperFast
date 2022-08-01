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

			// ResourceBundle __resourceBundleChain[2];
			size_t __resourceBundleCursor{};

			tf::Future<void> __updateJob;


			// Flags

			bool __swapchainDependencDirty{};
			bool __pipelineDependencyDirty{};
			bool __commandBufferDirty{};

			bool __allDrawcallSegmentDirty{};
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
			bool __isUpdateInFlight() const noexcept;

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

			void __resetSwapchainDependencies() noexcept;

			void __updateSwapchainDependencies();
			void __updatePipelineDependencies();
			void __updateCommandBuffers(tf::Taskflow *const pTaskFlow);

			void __checkSurfaceSupport() const;
			void __querySurfaceCapabilities() noexcept;
			void __querySupportedSurfaceFormats() noexcept;
			void __querySupportedSurfacePresentModes() noexcept;

			void __createSwapchain();
			void __createSwapchainImageViews();
			void __createRenderPass();
			void __createFramebuffer();
			void __buildPipelines(tf::Subflow &subflow);

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

			//constexpr void __swapResourceBundle() noexcept;

			/*[[nodiscard]]
			constexpr ResourceBundle &__getFrontResourceBundle() noexcept;

			[[nodiscard]]
			constexpr ResourceBundle &__getBackResourceBundle() noexcept;*/
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

	/*constexpr void ScreenManager::ScreenImpl::__swapResourceBundle() noexcept
	{
		__resourceBundleCursor = ((__resourceBundleCursor + 1ULL) % std::size(__resourceBundleChain));
	}*/

	/*constexpr ScreenManager::ScreenImpl::ResourceBundle &ScreenManager::ScreenImpl::__getFrontResourceBundle() noexcept
	{
		return __resourceBundleChain[__resourceBundleCursor];
	}

	constexpr ScreenManager::ScreenImpl::ResourceBundle &ScreenManager::ScreenImpl::__getBackResourceBundle() noexcept
	{
		const size_t backCursor{ (__resourceBundleCursor + 1ULL) % std::size(__resourceBundleChain) };
		return __resourceBundleChain[backCursor];
	}*/
}