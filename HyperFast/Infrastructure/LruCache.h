#pragma once

#include "Unique.h"
#include <list>
#include <unordered_map>

namespace Infra
{
	template <typename $Key, typename $Value>
	class LruCache : public Unique
	{
	public:
		LruCache(const size_t cacheSize) noexcept;

		[[nodiscard]]
		bool contains(const $Key &key) const noexcept;

		[[nodiscard]]
		$Value &get(const $Key &key) const noexcept;
		void set(const $Key &key, $Value &&value) noexcept;

		void clear() noexcept;

	private:
		class CacheInfo
		{
		public:
			std::list<$Key>::iterator referenceOrderIt;
			$Value value;
		};

		const size_t __cacheSize;
		std::list<$Key> __referenceOrder;
		std::unordered_map<$Key, CacheInfo> __cacheInfoMap;
	};

	template <typename $Key, typename $Value>
	LruCache<$Key, $Value>::LruCache(const size_t cacheSize) noexcept :
		__cacheSize{ cacheSize }
	{}

	template <typename $Key, typename $Value>
	bool LruCache<$Key, $Value>::contains(const $Key &key) const noexcept
	{
		return __cacheInfoMap.contains(key);
	}

	template <typename $Key, typename $Value>
	$Value &LruCache<$Key, $Value>::get(const $Key &key) const noexcept
	{
		return __cacheInfoMap.at(key).value;
	}

	template <typename $Key, typename $Value>
	void LruCache<$Key, $Value>::set(const $Key &key, $Value &&value) noexcept
	{
		const auto foundResult{ __cacheInfoMap.find(key) };
		if (foundResult != __cacheInfoMap.end())
		{
			CacheInfo &cacheInfo{ foundResult->second };
			cacheInfo.value = std::move(value);
			__referenceOrder.remove(cacheInfo.referenceOrderIt);
		}
		else if (__referenceOrder.size() == __cacheSize)
		{
			const $Key lruKey{ __referenceOrder.back() };
			__referenceOrder.pop_back();
			__cacheInfoMap.erase(lruKey);
		}

		__referenceOrder.emplace_front(key);

		CacheInfo &cacheInfo{ __cacheInfoMap[key] };
		cacheInfo.referenceOrderIt = __referenceOrder.begin();
		cacheInfo.value = std::move(value);
	}

	template <typename $Key, typename $Value>
	void LruCache<$Key, $Value>::clear() noexcept
	{
		__referenceOrder.clear();
		__cacheInfoMap.clear();
	}
}
