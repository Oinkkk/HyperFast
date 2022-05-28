#pragma once

#include "Unique.h"
#include <functional>
#include <memory>
#include <unordered_map>

namespace Infra
{
	template <typename ...$Args>
	class EventListener : public Unique
	{
	public:
		void setCallback(const std::function<void(const $Args &...)> &callback) noexcept;
		void send(const $Args &...args);

		[[nodiscard]]
		static std::shared_ptr<EventListener<$Args...>>
			make(const std::function<void(const $Args &...)> &callback) noexcept;

	private:
		std::function<void(const $Args &...)> __callbackFunc;
	};

	template <typename ...$Args>
	class EventView : public Unique
	{
	public:
		virtual void addListener(
			const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept = 0;
		
		virtual void removeListener(
			const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept = 0;

		EventView &operator+=(
			const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept;
		
		EventView &operator-=(
			const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept;
	};

	template <typename ...$Args>
	class Event : public EventView<$Args...>
	{
	public:
		virtual void addListener(
			const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept override;

		virtual void removeListener(
			const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept override;

		void invoke(const $Args &...args);

	private:
		std::unordered_map<
			EventListener<$Args...> *,
			std::weak_ptr<EventListener<$Args...>>> __listenerMap;
	};

	template <typename ...$Args>
	void EventListener<$Args...>::setCallback(const std::function<void(const $Args &...)> &callback) noexcept
	{
		__callbackFunc = callback;
	}

	template <typename ...$Args>
	void EventListener<$Args...>::send(const $Args &...args)
	{
		__callbackFunc(args...);
	}

	template <typename ...$Args>
	std::shared_ptr<EventListener<$Args...>>
		EventListener<$Args...>::make(const std::function<void(const $Args &...)> &callback) noexcept
	{
		const std::shared_ptr<EventListener<$Args...>> pRetVal
		{
			std::make_shared<EventListener<$Args...>>()
		};

		pRetVal->setCallback(callback);
		return pRetVal;
	}

	template <typename ...$Args>
	EventView<$Args...> &EventView<$Args...>::operator+=(
		const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept
	{
		addListener(pListener);
		return *this;
	}

	template <typename ...$Args>
	EventView<$Args...> &EventView<$Args...>::operator-=(
		const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept
	{
		removeListener(pListener);
		return *this;
	}

	template <typename ...$Args>
	void Event<$Args...>::addListener(
		const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept
	{
		__listenerMap.emplace(pListener.get(), pListener);
	}

	template <typename ...$Args>
	void Event<$Args...>::removeListener(
		const std::shared_ptr<EventListener<$Args...>> &pListener) noexcept
	{
		__listenerMap.erase(pListener.get());
	}

	template <typename ...$Args>
	void Event<$Args...>::invoke(const $Args &...args)
	{
		for (auto iter = __listenerMap.begin(); iter != __listenerMap.end(); )
		{
			const auto &[pListener, wpListener]{ *iter };

			if (wpListener.expired())
				iter = __listenerMap.erase(iter);
			else
			{
				pListener->send(args...);
				iter++;
			}
		}
	}
}
