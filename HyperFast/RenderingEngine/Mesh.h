#pragma once

#include "Buffer.h"
#include "VertexAttribute.h"

namespace HyperFast
{
	class Mesh : public Infra::Unique
	{
	public:
		Mesh(VKL::DeviceProcedure &deviceProc) noexcept;

		void setPositionBuffer(const std::shared_ptr<Buffer> &pBuffer) noexcept;
		void setColorBuffer(const std::shared_ptr<Buffer> &pBuffer) noexcept;

		[[nodiscard]]
		constexpr VertexAttributeFlag getVertexAttributeFlag() const noexcept;

		void bind(const VkCommandBuffer commandBuffer) const noexcept;

	private:
		VKL::DeviceProcedure &__deviceProc;

		std::shared_ptr<Buffer> __pPositionBuffer;
		std::shared_ptr<Buffer> __pColorBuffer;

		VertexAttributeFlag __attribFlag{};

		VkBuffer __handles[VERTEX_ATTRIB_LOCATION_MAX]{};
		VkDeviceSize __offsets[VERTEX_ATTRIB_LOCATION_MAX]{};

		constexpr void __setAttribFlagBit(const VertexAttributeFlagBit flagBit, const bool set) noexcept;
		constexpr void __setHandle(const uint32_t attribLocation, Buffer *const pBuffer) noexcept;

		void __setBuffer(
			const VertexAttributeFlagBit attribFlagBit, const uint32_t attribLocation,
			std::shared_ptr<Buffer> &pOldBuffer, const std::shared_ptr<Buffer> &pNewBuffer) noexcept;
	};

	constexpr VertexAttributeFlag Mesh::getVertexAttributeFlag() const noexcept
	{
		return __attribFlag;
	}

	constexpr void Mesh::__setAttribFlagBit(const VertexAttributeFlagBit flagBit, const bool set) noexcept
	{
		if (set)
			__attribFlag |= flagBit;
		else
			__attribFlag &= ~flagBit;
	}

	constexpr void Mesh::__setHandle(const uint32_t attribLocation, Buffer *const pBuffer) noexcept
	{
		VkBuffer handle{};
		if (pBuffer)
			handle = pBuffer->getHandle();

		__handles[attribLocation] = handle;
	}
}
