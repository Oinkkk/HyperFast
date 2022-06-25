#include "Submesh.h"

namespace HyperFast
{
	Submesh::Submesh(const std::shared_ptr<Mesh> &pMesh, const VKL::DeviceProcedure &deviceProc) noexcept :
		__pMesh{ pMesh }, __deviceProc{ deviceProc }
	{}

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

	Mesh &Submesh::getMesh() const noexcept
	{
		return *__pMesh;
	}

	void Submesh::draw(const VkCommandBuffer commandBuffer) const noexcept
	{
		__deviceProc.vkCmdDrawIndexed(
			commandBuffer, __drawCommand.indexCount, __drawCommand.instanceCount,
			__drawCommand.firstIndex, __drawCommand.vertexOffset, __drawCommand.firstInstance);
	}
}