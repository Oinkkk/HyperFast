#include "ScreenManager.h"
#include "RenderingEngine.h"

namespace HyperFast
{
	ScreenManager::ScreenImpl::ScreenImpl(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t queueFamilyIndex, Vulkan::Device &device,
		Vulkan::Queue &queue, LifeCycle &lifeCycle,
		CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter, Win::Window &window) :
		__instance{ instance }, __physicalDevice{ physicalDevice },
		__queueFamilyIndex{ queueFamilyIndex }, __device{ device },
		__queue{ queue }, __lifeCycle{ lifeCycle },
		__commandSubmitter{ commandSubmitter }, __resourceDeleter{ resourceDeleter },
		__window{ window }
	{
		__initListeners();
		__registerListeners();
		__createSurface();
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__destroy();
	}

	void ScreenManager::ScreenImpl::setDrawcall(Drawcall *const pDrawcall) noexcept
	{
		Drawcall *&pCurDrawcall{ __pDrawcall };
		Drawcall *const pNewDrawcall{ pDrawcall };

		if (pCurDrawcall)
		{
			pCurDrawcall->getMeshBufferChangeEvent() -= __pDrawcallMeshBufferChangeEventListener;
			pCurDrawcall->getIndirectBufferUpdateEvent() -= __pDrawcallIndirectBufferUpdateEventListener;
			pCurDrawcall->getIndirectBufferCreateEvent() -= __pDrawcallIndirectBufferCreateEventListener;
		}

		if (pNewDrawcall)
		{
			pNewDrawcall->getMeshBufferChangeEvent() += __pDrawcallMeshBufferChangeEventListener;
			pNewDrawcall->getIndirectBufferUpdateEvent() += __pDrawcallIndirectBufferUpdateEventListener;
			pNewDrawcall->getIndirectBufferCreateEvent() += __pDrawcallIndirectBufferCreateEventListener;
		}

		pCurDrawcall = pNewDrawcall;
		__needToUpdatePipelineDependencies = true;
	}

	bool ScreenManager::ScreenImpl::__isValid() const noexcept
	{
		if (__destroyed)
			return false;

		const bool validSize{ __window.getWidth() && __window.getHeight() };
		return validSize;
	}

	bool ScreenManager::ScreenImpl::__isRenderable() const noexcept
	{
		if (!(__isValid()))
			return false;

		// TODO: 추가 로직

		return true;
	}

	void ScreenManager::ScreenImpl::__update()
	{
		if (__needToUpdateSwapchainDependencies)
			__updateSwapchainDependencies();

		if (__needToUpdatePipelineDependencies)
			__updatePipelineDependencies();

		if (__needToUpdateCommandBuffers)
			__updateCommandBuffers();
	}

	void ScreenManager::ScreenImpl::__render() noexcept
	{
		// TODO: 렌더링 로직

		__needToRender = false;
		__needToPresent = true;
	}

	void ScreenManager::ScreenImpl::__present() noexcept
	{
		// TODO: present 로직
	}

	void ScreenManager::ScreenImpl::__destroy() noexcept
	{
		if (__destroyed)
			return;

		// TODO: 파괴로직
	
		__destroyed = true;
	}

	void ScreenManager::ScreenImpl::__initListeners() noexcept
	{
		__pWindowResizeEventListener =
			Infra::EventListener<Win::Window &, Win::Window::ResizingType>::bind(
				&ScreenManager::ScreenImpl::__onWindowResize, this,
				std::placeholders::_1, std::placeholders::_2);

		__pWindowDrawEventListener =
			Infra::EventListener<Win::Window &>::bind(
				&ScreenManager::ScreenImpl::__onWindowDraw, this, std::placeholders::_1);

		__pWindowDestroyEventListener =
			Infra::EventListener<Win::Window &>::bind(
				&ScreenManager::ScreenImpl::__onWindowDestroy, this, std::placeholders::_1);

		__pDrawcallMeshBufferChangeEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager::ScreenImpl::__onDrawcallMeshBufferChange, this,
				std::placeholders::_1, std::placeholders::_2);

		__pDrawcallIndirectBufferUpdateEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager::ScreenImpl::__onDrawcallIndirectBufferUpdate, this,
				std::placeholders::_1, std::placeholders::_2);

		__pDrawcallIndirectBufferCreateEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager::ScreenImpl::__onDrawcallIndirectBufferCreate, this,
				std::placeholders::_1, std::placeholders::_2);

		__pScreenUpdateListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onScreenUpdate, this);

		__pRenderListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onRender, this);

		__pPresentListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onPresent, this);
	}

	void ScreenManager::ScreenImpl::__registerListeners() noexcept
	{
		__window.getResizeEvent() += __pWindowResizeEventListener;
		__window.getDrawEvent() += __pWindowDrawEventListener;
		__window.getDestroyEvent() += __pWindowDestroyEventListener;

		__lifeCycle.getSignalEvent(LifeCycleType::SCREEN_UPDATE) += __pScreenUpdateListener;
		__lifeCycle.getSignalEvent(LifeCycleType::RENDER) += __pRenderListener;
		__lifeCycle.getSignalEvent(LifeCycleType::PRESENT) += __pPresentListener;
	}

	void ScreenManager::ScreenImpl::__createSurface()
	{
		const VkWin32SurfaceCreateInfoKHR createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = __window.getClass().getHInstance(),
			.hwnd = __window.getHandle()
		};

		__pSurface = std::make_unique<Vulkan::Surface>(__instance, createInfo);
		__needToUpdateSwapchainDependencies = true;
	}

	void ScreenManager::ScreenImpl::__updateSwapchainDependencies()
	{
		// TODO: 스왑체인 업데이트


		__updatePipelineDependencies();
		__needToUpdateSwapchainDependencies = false;
	}

	void ScreenManager::ScreenImpl::__updatePipelineDependencies()
	{
		// TODO: 파이프라인 업데이트
		__updateCommandBuffers();
		__needToUpdatePipelineDependencies = false;
	}

	void ScreenManager::ScreenImpl::__updateCommandBuffers()
	{
		// TODO: 커맨드 버퍼 업데이트
		__needToUpdateCommandBuffers = false;
		__needToRender = true;
	}

	void ScreenManager::ScreenImpl::__onWindowResize(
		Win::Window &window, const Win::Window::ResizingType resizingType) noexcept
	{
		if (resizingType == Win::Window::ResizingType::MINIMIZED)
			return;

		__needToUpdateSwapchainDependencies = true;
	}

	void ScreenManager::ScreenImpl::__onDrawcallMeshBufferChange(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__needToUpdateCommandBuffers = true;
	}

	void ScreenManager::ScreenImpl::__onDrawcallIndirectBufferUpdate(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__needToRender = true;
	}

	void ScreenManager::ScreenImpl::__onDrawcallIndirectBufferCreate(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__needToUpdateCommandBuffers = true;
	}

	void ScreenManager::ScreenImpl::__onWindowDraw(Win::Window &window) noexcept
	{
		__needToRender = true;
	}

	void ScreenManager::ScreenImpl::__onWindowDestroy(Win::Window &window) noexcept
	{
		__destroy();
	}

	void ScreenManager::ScreenImpl::__onScreenUpdate()
	{
		if (!(__isValid()))
			return;

		__update();
	}

	void ScreenManager::ScreenImpl::__onRender() noexcept
	{
		if (!__needToRender)
			return;

		if (!(__isRenderable()))
			return;

		__render();
	}

	void ScreenManager::ScreenImpl::__onPresent() noexcept
	{
		if (!__needToPresent)
			return;

		if (!(__isValid()))
			return;

		__present();
	}
}