#include "TemporalDeleter.h"

namespace Infra
{
	TemporalDeleter::TemporalDeleter() noexcept :
		__pCurrentDeleter{ new Deleter() }
	{}

	TemporalDeleter::~TemporalDeleter() noexcept
	{
		for (Deleter *const pDeleter : __pendingDeleters)
			delete pDeleter;

		delete __pCurrentDeleter;

		for (Deleter *const pDeleter : __idleDeleters)
			delete pDeleter;
	}

	void TemporalDeleter::advanceTimestamp() noexcept
	{
		__pendingDeleters.emplace_back(__pCurrentDeleter);

		if (__idleDeleters.empty())
			__pCurrentDeleter = new Deleter();
		else
		{
			__pCurrentDeleter = __idleDeleters.front();
			__idleDeleters.pop_front();
		}
	}

	void TemporalDeleter::reserve(Deletable *const pDeletable) noexcept
	{
		__pCurrentDeleter->reserve(pDeletable);
	}

	void TemporalDeleter::commit(const size_t timestamp)
	{
		for (; __oldestTimestamp <= timestamp; __oldestTimestamp++)
		{
			Deleter *const pDeleter{ __pendingDeleters.front() };
			pDeleter->commit();

			__pendingDeleters.pop_front();
			__idleDeleters.emplace_back(pDeleter);
		}
	}
}