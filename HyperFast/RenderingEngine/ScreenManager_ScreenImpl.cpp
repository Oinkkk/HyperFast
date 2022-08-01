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
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__destroy();
	}

	void ScreenManager::ScreenImpl::setDrawcall(Drawcall *const pDrawcall) noexcept
	{
		
	}

	bool ScreenManager::ScreenImpl::__isValid() const noexcept
	{
		if (__destroyed)
			return false;

		const bool validSize{ __window.getWidth() && __window.getHeight() };
		return validSize;
	}

	void ScreenManager::ScreenImpl::__update()
	{
		
	}

	void ScreenManager::ScreenImpl::__render() noexcept
	{
		
	}

	void ScreenManager::ScreenImpl::__present() noexcept
	{
		
	}

	void ScreenManager::ScreenImpl::__destroy() noexcept
	{
		if (__destroyed)
			return;

		
	
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

	void ScreenManager::ScreenImpl::__onWindowResize(
		Win::Window &window, const Win::Window::ResizingType resizingType) noexcept
	{
		
	}

	void ScreenManager::ScreenImpl::__onDrawcallMeshBufferChange(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		
	}

	void ScreenManager::ScreenImpl::__onDrawcallIndirectBufferUpdate(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		
	}

	void ScreenManager::ScreenImpl::__onDrawcallIndirectBufferCreate(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		
	}

	void ScreenManager::ScreenImpl::__onWindowDraw(Win::Window &window) noexcept
	{
		
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
		
	}

	void ScreenManager::ScreenImpl::__onPresent() noexcept
	{
		
	}
}