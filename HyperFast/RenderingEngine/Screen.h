#pragma once

#include "../Infrastructure/ResourceAllocator.h"
#include <vulkan/vulkan.h>
#include "../Window/Window.h"

namespace HyperFast
{
	class SurfaceAllocator : public Infra::ResourceAllocator<VkSurfaceKHR>
	{
	public:
		virtual VkSurfaceKHR allocate(Win::Window &window) = 0;
	};

	class Screen : public Infra::Unique
	{
	public:
		Screen(SurfaceAllocator &surfaceAllocator, Win::Window &window);
		virtual ~Screen() noexcept;

		void destroy() noexcept;

		[[nodiscard]]
		constexpr bool isDestroyed() const noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Screen &> &getDestroyEvent() noexcept;

	private:
		SurfaceAllocator &__surfaceAllocator;
		Win::Window &__window;

		VkSurfaceKHR __surface{};

		Infra::Event<Screen &> __destroyEvent;
		std::shared_ptr<Infra::EventListener<Win::Window &>> __pDestroyEventListener;
	};

	constexpr bool Screen::isDestroyed() const noexcept
	{
		return __surface;
	}

	constexpr Infra::EventView<Screen &> &Screen::getDestroyEvent() noexcept
	{
		return __destroyEvent;
	}
}
