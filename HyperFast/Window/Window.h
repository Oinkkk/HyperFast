#pragma once

#include "WindowClass.h"
#include <string_view>
#include "../Infrastructure/Event.h"
#include "EventConsumption.h"

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
			const WindowClass &windowClass, const std::string_view &title, const bool show);
		
		Window(
			const WindowClass &windowClass, const std::string_view &title,
			const bool show, const int x, const int y);

		Window(
			const WindowClass &windowClass, const std::string_view &title,
			const bool show, const int x, const int y, const int width, const int height);

		virtual ~Window() noexcept;

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
		void validate() noexcept;

		void destroy();

		LRESULT sendRawMessage(const UINT uMsg, const WPARAM wParam, const LPARAM lParam);

		[[nodiscard]]
		constexpr Infra::EventView<Window *, EventConsumption *, ResizingType> &getResizeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Window *, EventConsumption *> &getDrawEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Window *, EventConsumption *> &getCloseEvent() noexcept;

	private:
		HWND __handle{};
		RECT __windowRect{};
		RECT __clientRect{};

		EventConsumptionImpl __eventConsumption;
		Infra::Event<Window *, EventConsumption *, ResizingType> __resizeEvent;
		Infra::Event<Window *, EventConsumption *> __drawEvent;
		Infra::Event<Window *, EventConsumption *> __closeEvent;

		[[nodiscard]]
		constexpr int __getWindowWidth() const noexcept;

		[[nodiscard]]
		constexpr int __getWindowHeight() const noexcept;

		void __updateAppearance(const int x, const int y, const int width, const int height) noexcept;

		static HWND __create(
			const WindowClass &windowClass, const std::string_view &title,
			const int x, const int y, const int width, const int height, Window *const pThis);
	};

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

	constexpr Infra::EventView<Window *, EventConsumption *, Window::ResizingType> &Window::getResizeEvent() noexcept
	{
		return __resizeEvent;
	}

	constexpr Infra::EventView<Window *, EventConsumption *> &Window::getDrawEvent() noexcept
	{
		return __drawEvent;
	}

	constexpr Infra::EventView<Window *, Win::EventConsumption *> &Window::getCloseEvent() noexcept
	{
		return __closeEvent;
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
