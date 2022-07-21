#pragma once

#include "Buffer.h"
#include "VertexAttribute.h"
#include "../Infrastructure/Event.h"
#include "../Vulkan/CommandBuffer.h"

namespace HyperFast
{
	class Mesh : public Infra::Unique
	{
	public:
		Mesh(Vulkan::Device &device) noexcept;

		[[nodiscard]]
		Buffer *getPositionBuffer() const noexcept;
		void setPositionBuffer(std::unique_ptr<Buffer> &&pBuffer) noexcept;

		[[nodiscard]]
		Buffer *getColorBuffer() const noexcept;
		void setColorBuffer(std::unique_ptr<Buffer> &&pBuffer) noexcept;

		[[nodiscard]]
		Buffer *getIndexBuffer() const noexcept;
		void setIndexBuffer(std::unique_ptr<Buffer> &&pBuffer, const VkIndexType indexType) noexcept;

		[[nodiscard]]
		constexpr VertexAttributeFlag getVertexAttributeFlag() const noexcept;
		void bind(Vulkan::CommandBuffer &commandBuffer) const noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Mesh &, VertexAttributeFlag, VertexAttributeFlag> &
			getAttributeFlagChangeEvent() noexcept;

		void addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept;

	private:
		Vulkan::Device &__device;

		std::unique_ptr<Buffer> __pPositionBuffer;
		std::unique_ptr<Buffer> __pColorBuffer;
		std::unique_ptr<Buffer> __pIndexBuffer;
		VkIndexType __indexType{};

		VertexAttributeFlag __attribFlag{};

		VkBuffer __handles[VERTEX_ATTRIB_LOCATION_MAX]{};
		VkDeviceSize __offsets[VERTEX_ATTRIB_LOCATION_MAX]{};

		Infra::Event<Mesh &, VertexAttributeFlag, VertexAttributeFlag> __attribFlagChangeEvent;

		constexpr void __setAttribFlagBit(const VertexAttributeFlagBit flagBit, const bool set) noexcept;
		constexpr void __setHandle(const uint32_t attribLocation, Buffer *const pBuffer) noexcept;

		void __setBuffer(
			const VertexAttributeFlagBit attribFlagBit, const uint32_t attribLocation,
			std::unique_ptr<Buffer> &pOldBuffer, std::unique_ptr<Buffer> &&pNewBuffer) noexcept;
	};

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