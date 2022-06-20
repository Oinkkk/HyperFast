#include "Mesh.h"

namespace HyperFast
{
	Mesh::Mesh(VKL::DeviceProcedure &deviceProc) noexcept :
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

	void Mesh::bind(const VkCommandBuffer commandBuffer) const noexcept
	{
		__deviceProc.vkCmdBindVertexBuffers(
			commandBuffer, 0U, VERTEX_ATTRIB_LOCATION_MAX, __handles, __offsets);
	}

	void Mesh::__setBuffer(
		const VertexAttributeFlagBit attribFlagBit, const uint32_t attribLocation,
		std::shared_ptr<Buffer> &pOldBuffer, const std::shared_ptr<Buffer> &pNewBuffer) noexcept
	{
		pOldBuffer = pNewBuffer;

		Buffer *const pRaw{ pNewBuffer.get() };
		__setAttribFlagBit(attribFlagBit, pRaw);
		__setHandle(attribLocation, pRaw);
	}
}