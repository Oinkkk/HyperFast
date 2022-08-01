#pragma once

#include "../Infrastructure/TemporalDeleter.h"
#include "../Window/Window.h"
#include "../Vulkan/Queue.h"
#include "../Vulkan/Fence.h"
#include "../Vulkan/Surface.h"
#include "../Vulkan/Swapchain.h"
#include "LifeCycle.h"
#include "CommandSubmitter.h"
#include "Drawcall.h"

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

			std::unique_ptr<Vulkan::Surface> __pSurface;
			Vulkan::Swapchain *__pSwapchain{};


			// Flags

			bool __needToUpdateSwapchainDependencies{};
			bool __needToUpdatePipelineDependencies{};
			bool __needToUpdateCommandBuffers{};

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
			bool __isValid() const noexcept;

			[[nodiscard]]
			bool __isRenderable() const noexcept;

			void __update();
			void __render() noexcept;
			void __present() noexcept;

			void __destroy() noexcept;

			void __initListeners() noexcept;
			void __registerListeners() noexcept;
			void __createSurface();

			void __updateSwapchainDependencies();
			void __updatePipelineDependencies();
			void __updateCommandBuffers();

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
}