#pragma once

#include "Submesh.h"
#include "../Infrastructure/IdAllocator.h"

namespace HyperFast
{
	class IndirectBufferBuilder : public Infra::Unique
	{
	public:
		void addSubmesh(Submesh &submesh) noexcept;
		void removeSubmesh(Submesh &submesh) noexcept;

	private:
		Infra::IdAllocator __idAllocator;
	};
}
