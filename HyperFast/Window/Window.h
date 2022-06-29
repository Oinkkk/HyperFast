#pragma once

#include "WindowClass.h"
#include <string_view>
#include "../Infrastructure/Event.h"

namespace Win
{
	class Window : public Infra::Unique
	{
	public:
		enum class ResizingType : uint32_t
		{
			NORMAL		= SIZE_RESTORED,
			MINIMIZED	= SIZE_MINIMIZED,
			MAXIMIZED	= SIZE_MAXIMIZED,
			REVEALED	= SIZE_MAXSHOW,
			COVERED		= SIZE_MAXHIDE
		};

		Window(
			WindowClass &windowClass, const std::string_view &title, const bool show);
		
		Window(
			WindowClass &windowClass, const std::string_view &title,
			const bool show, const int x, const int y);

		Window(
			WindowClass &windowClass, const std::string_view &title,
			const bool show, const int x, const int y, const int width, const int height);

		virtual ~Window() noexcept;

		[[nodiscard]]
		constexpr WindowClass &getClass() const noexcept;

		[[nodiscard]]
		constexpr HWND getHandle() const noexcept;

		void setShow(const bool show = true) noexcept;

		[[nodiscard]]
		constexpr int getX() const noexcept;
		void setX(const int x) noexcept;

		[[nodiscard]]
		constexpr int getY() const noexcept;
		void setY(const int y) noexcept;

		void setPosition(const int x, const int y) noexcept;

		[[nodiscard]]
		constexpr int getWidth() const noexcept;
		void setWidth(const int x) noexcept;

		[[nodiscard]]
		constexpr int getHeight() const noexcept;
		void setHeight(const int y) noexcept;

		void setSize(const int width, const int height) noexcept;

		void invalidate() noexcept;
		void destroy();

		[[nodiscard]]
		constexpr bool isDestroyed() const noexcept;

		LRESULT sendRawMessage(const UINT uMsg, const WPARAM wParam, const LPARAM lParam);

		[[nodiscard]]
		constexpr Infra::EventView<Window &, ResizingType> &getResizeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Window &> &getDrawEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Window &> &getDestroyEvent() noexcept;

	private:
		WindowClass &__windowClass;

		HWND __handle{};
		RECT __windowRect{};
		RECT __clientRect{};

		Infra::Event<Window &, ResizingType> __resizeEvent;
		Infra::Event<Window &> __drawEvent;
		Infra::Event<Window &> __destroyEvent;

		[[nodiscard]]
		constexpr int __getWindowWidth() const noexcept;

		[[nodiscard]]
		constexpr int __getWindowHeight() const noexcept;

		void __updateAppearance(const int x, const int y, const int windowWidth, const int windowHeight) noexcept;

		static HWND __create(
			WindowClass &windowClass, const std::string_view &title,
			const int x, const int y, const int width, const int height, Window *const pThis);
	};

	[[nodiscard]]
	constexpr WindowClass &Window::getClass() const noexcept
	{
		return __windowClass;
	}

	constexpr HWND Window::getHandle() const noexcept
	{
		return __handle;
	}

	constexpr int Window::getX() const noexcept
	{
		return __windowRect.left;
	}

	constexpr int Window::getY() const noexcept
	{
		return __windowRect.top;
	}

	constexpr int Window::getWidth() const noexcept
	{
		return { __clientRect.right - __clientRect.left };
	}

	constexpr int Window::getHeight() const noexcept
	{
		return { __clientRect.bottom - __clientRect.top };
	}

	constexpr bool Window::isDestroyed() const noexcept
	{
		return __handle;
	}

	constexpr Infra::EventView<Window &, Window::ResizingType> &Window::getResizeEvent() noexcept
	{
		return __resizeEvent;
	}

	constexpr Infra::EventView<Window &> &Window::getDrawEvent() noexcept
	{
		return __drawEvent;
	}

	constexpr Infra::EventView<Window &> &Window::getDestroyEvent() noexcept
	{
		return __destroyEvent;
	}

	constexpr int Window::__getWindowWidth() const noexcept
	{
		return { __windowRect.right - __windowRect.left };
	}

	constexpr int Window::__getWindowHeight() const noexcept
	{
		return { __windowRect.bottom - __windowRect.top };
	}
}
