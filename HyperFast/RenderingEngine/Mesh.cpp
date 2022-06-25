#include "Mesh.h"

namespace HyperFast
{
	Mesh::Mesh(const VKL::DeviceProcedure &deviceProc) noexcept :
		__deviceProc{ deviceProc }
	{}

	void Mesh::setPositionBuffer(const std::shared_ptr<Buffer> &pBuffer) noexcept
	{
		__setBuffer(
			VertexAttributeFlagBit::POS3, VERTEX_ATTRIB_LOCATION_POS,
			__pPositionBuffer, pBuffer);
	}

	void Mesh::setColorBuffer(const std::shared_ptr<Buffer> &pBuffer) noexcept
	{
		__setBuffer(
			VertexAttributeFlagBit::COLOR4, VERTEX_ATTRIB_LOCATION_COLOR,
			__pColorBuffer, pBuffer);
	}

	void Mesh::setIndexBuffer(const std::shared_ptr<Buffer> &pBuffer, const VkIndexType indexType) noexcept
	{
		__pIndexBuffer = pBuffer;
		__indexType = indexType;
	}

	void Mesh::bind(const VkCommandBuffer commandBuffer) const noexcept
	{
		__deviceProc.vkCmdBindVertexBuffers(
			commandBuffer, 0U, VERTEX_ATTRIB_LOCATION_MAX, __handles, __offsets);

		__deviceProc.vkCmdBindIndexBuffer(
			commandBuffer, __pIndexBuffer->getHandle(), 0ULL, __indexType);
	}

	void Mesh::__setBuffer(
		const VertexAttributeFlagBit attribFlagBit, const uint32_t attribLocation,
		std::shared_ptr<Buffer> &pOldBuffer, const std::shared_ptr<Buffer> &pNewBuffer) noexcept
	{
		const VertexAttributeFlag prevAttribFlag{ __attribFlag };

		pOldBuffer = pNewBuffer;
		Buffer *const pRaw{ pNewBuffer.get() };

		__setAttribFlagBit(attribFlagBit, pRaw);
		__setHandle(attribLocation, pRaw);

		if (prevAttribFlag != __attribFlag)
			__attribFlagChangeEvent.invoke(*this, prevAttribFlag, __attribFlag);
	}
}