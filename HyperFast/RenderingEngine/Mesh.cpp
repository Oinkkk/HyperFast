#include "Mesh.h"

namespace HyperFast
{
	Mesh::Mesh(Vulkan::Device &device) noexcept :
		__device{ device }
	{}

	Mesh::~Mesh() noexcept
	{
		__destroyEvent.invoke(*this);
	}

	Buffer *Mesh::getPositionBuffer() const noexcept
	{
		return __pPositionBuffer.get();
	}

	void Mesh::setPositionBuffer(std::unique_ptr<Buffer> &&pBuffer) noexcept
	{
		__setBuffer(
			VertexAttributeFlagBit::POS3, VERTEX_ATTRIB_LOCATION_POS,
			__pPositionBuffer, std::move(pBuffer));
	}

	Buffer *Mesh::getColorBuffer() const noexcept
	{
		return __pColorBuffer.get();
	}

	void Mesh::setColorBuffer(std::unique_ptr<Buffer> &&pBuffer) noexcept
	{
		__setBuffer(
			VertexAttributeFlagBit::COLOR4, VERTEX_ATTRIB_LOCATION_COLOR,
			__pColorBuffer, std::move(pBuffer));
	}

	Buffer *Mesh::getIndexBuffer() const noexcept
	{
		return __pIndexBuffer.get();
	}

	void Mesh::setIndexBuffer(std::unique_ptr<Buffer> &&pBuffer, const VkIndexType indexType) noexcept
	{
		__pIndexBuffer = std::move(pBuffer);
		__indexType = indexType;
	}

	void Mesh::bind(Vulkan::CommandBuffer &commandBuffer) const noexcept
	{
		commandBuffer.vkCmdBindVertexBuffers(
			0U, VERTEX_ATTRIB_LOCATION_MAX, __handles, __offsets);

		commandBuffer.vkCmdBindIndexBuffer(
			__pIndexBuffer->getHandle(), 0ULL, __indexType);
	}

	void Mesh::addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		__pPositionBuffer->addSemaphoreDependency(pDependency);
		__pColorBuffer->addSemaphoreDependency(pDependency);
		__pIndexBuffer->addSemaphoreDependency(pDependency);
	}

	void Mesh::__setBuffer(
		const VertexAttributeFlagBit attribFlagBit, const uint32_t attribLocation,
		std::unique_ptr<Buffer> &pOldBuffer, std::unique_ptr<Buffer> &&pNewBuffer) noexcept
	{
		const VertexAttributeFlag prevAttribFlag{ __attribFlag };

		pOldBuffer = std::move(pNewBuffer);
		Buffer *const pRaw{ pOldBuffer.get() };

		__setAttribFlagBit(attribFlagBit, pRaw);
		__setHandle(attribLocation, pRaw);

		if (prevAttribFlag != __attribFlag)
			__attribFlagChangeEvent.invoke(*this, prevAttribFlag, __attribFlag);

		__bufferChangeEvent.invoke(*this);
	}
}