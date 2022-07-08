#include "Submesh.h"

namespace HyperFast
{
	Submesh::Submesh(const std::shared_ptr<Mesh> &pMesh, const Vulkan::DeviceProcedure &deviceProc) noexcept :
		__pMesh{ pMesh }, __deviceProc{ deviceProc }
	{}

	Submesh::~Submesh() noexcept
	{
		__destroyEvent.invoke(*this);
	}

	void Submesh::setIndexCount(const uint32_t indexCount) noexcept
	{
		if (__drawCommand.indexCount == indexCount)
			return;

		__drawCommand.indexCount = indexCount;
		__drawCommandChangeEvent.invoke(*this);
	}

	void Submesh::setInstanceCount(const uint32_t instanceCount) noexcept
	{
		if (__drawCommand.instanceCount == instanceCount)
			return;

		__drawCommand.instanceCount = instanceCount;
		__drawCommandChangeEvent.invoke(*this);
	}

	void Submesh::setFirstIndex(const uint32_t firstIndex) noexcept
	{
		if (__drawCommand.firstIndex == firstIndex)
			return;

		__drawCommand.firstIndex = firstIndex;
		__drawCommandChangeEvent.invoke(*this);
	}

	void Submesh::setVertexOffset(const int32_t vertexOffset) noexcept
	{
		if (__drawCommand.vertexOffset == vertexOffset)
			return;

		__drawCommand.vertexOffset = vertexOffset;
		__drawCommandChangeEvent.invoke(*this);
	}

	void Submesh::setFirstInstance(const uint32_t firstInstance) noexcept
	{
		if (__drawCommand.firstInstance == firstInstance)
			return;

		__drawCommand.firstInstance = firstInstance;
		__drawCommandChangeEvent.invoke(*this);
	}

	void Submesh::setDrawCommand(
		const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex,
		const int32_t vertexOffset, const uint32_t firstInstance) noexcept
	{
		bool changed{};

		if (__drawCommand.indexCount != indexCount)
		{
			__drawCommand.indexCount = indexCount;
			changed = true;
		}

		if (__drawCommand.instanceCount != instanceCount)
		{
			__drawCommand.instanceCount = instanceCount;
			changed = true;
		}

		if (__drawCommand.firstIndex != firstIndex)
		{
			__drawCommand.firstIndex = firstIndex;
			changed = true;
		}

		if (__drawCommand.vertexOffset != vertexOffset)
		{
			__drawCommand.vertexOffset = vertexOffset;
			changed = true;
		}

		if (__drawCommand.firstInstance != firstInstance)
		{
			__drawCommand.firstInstance = firstInstance;
			changed = true;
		}

		if (changed)
			__drawCommandChangeEvent.invoke(*this);
	}

	void Submesh::setVisible(const bool visible) noexcept
	{
		if (__visible == visible)
			return;

		__visible = visible;
		__visibleChangeEvent.invoke(*this);
	}

	Mesh &Submesh::getMesh() const noexcept
	{
		return *__pMesh;
	}
}