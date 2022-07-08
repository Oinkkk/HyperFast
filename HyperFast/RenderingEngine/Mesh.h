#pragma once

#include "Buffer.h"
#include "VertexAttribute.h"
#include "../Infrastructure/Event.h"

namespace HyperFast
{
	class Mesh : public Infra::Unique
	{
	public:
		Mesh(const Vulkan::DeviceProcedure &deviceProc) noexcept;

		[[nodiscard]]
		constexpr const std::shared_ptr<Buffer> &getPositionBuffer() const noexcept;
		void setPositionBuffer(const std::shared_ptr<Buffer> &pBuffer) noexcept;

		[[nodiscard]]
		constexpr const std::shared_ptr<Buffer> &getColorBuffer() const noexcept;
		void setColorBuffer(const std::shared_ptr<Buffer> &pBuffer) noexcept;

		[[nodiscard]]
		constexpr const std::shared_ptr<Buffer> &getIndexBuffer() const noexcept;
		void setIndexBuffer(const std::shared_ptr<Buffer> &pBuffer, const VkIndexType indexType) noexcept;

		[[nodiscard]]
		constexpr VertexAttributeFlag getVertexAttributeFlag() const noexcept;
		void bind(const VkCommandBuffer commandBuffer) const noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Mesh &, VertexAttributeFlag, VertexAttributeFlag> &getAttributeFlagChangeEvent() noexcept;

	private:
		const Vulkan::DeviceProcedure &__deviceProc;

		std::shared_ptr<Buffer> __pPositionBuffer;
		std::shared_ptr<Buffer> __pColorBuffer;

		std::shared_ptr<Buffer> __pIndexBuffer;
		VkIndexType __indexType{};

		VertexAttributeFlag __attribFlag{};

		VkBuffer __handles[VERTEX_ATTRIB_LOCATION_MAX]{};
		VkDeviceSize __offsets[VERTEX_ATTRIB_LOCATION_MAX]{};

		Infra::Event<Mesh &, VertexAttributeFlag, VertexAttributeFlag> __attribFlagChangeEvent;

		constexpr void __setAttribFlagBit(const VertexAttributeFlagBit flagBit, const bool set) noexcept;
		constexpr void __setHandle(const uint32_t attribLocation, Buffer *const pBuffer) noexcept;

		void __setBuffer(
			const VertexAttributeFlagBit attribFlagBit, const uint32_t attribLocation,
			std::shared_ptr<Buffer> &pOldBuffer, const std::shared_ptr<Buffer> &pNewBuffer) noexcept;
	};

	constexpr const std::shared_ptr<Buffer> &Mesh::getPositionBuffer() const noexcept
	{
		return __pPositionBuffer;
	}

	constexpr const std::shared_ptr<Buffer> &Mesh::getColorBuffer() const noexcept
	{
		return __pColorBuffer;
	}

	constexpr const std::shared_ptr<Buffer> &Mesh::getIndexBuffer() const noexcept
	{
		return __pIndexBuffer;
	}

	constexpr VertexAttributeFlag Mesh::getVertexAttributeFlag() const noexcept
	{
		return __attribFlag;
	}

	constexpr Infra::EventView<Mesh &, VertexAttributeFlag, VertexAttributeFlag> &
		Mesh::getAttributeFlagChangeEvent() noexcept
	{
		return __attribFlagChangeEvent;
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