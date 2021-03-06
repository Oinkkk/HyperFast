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
		~Mesh() noexcept;

		[[nodiscard]]
		Buffer *getPositionBuffer() const noexcept;
		void setPositionBuffer(std::unique_ptr<Buffer> &&pBuffer) noexcept;

		[[nodiscard]]
		Buffer *getColorBuffer() const noexcept;
		void setColorBuffer(std::unique_ptr<Buffer> &&pBuffer) noexcept;

		[[nodiscard]]
		Buffer *getIndexBuffer() const noexcept;
		void setIndexBuffer(std::unique_ptr<Buffer> &&pBuffer, const VkIndexType indexType) noexcept;

		void bind(Vulkan::CommandBuffer &commandBuffer) const noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Mesh &> &getBufferChangeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Mesh &> &getDestroyEvent() noexcept;

	private:
		Vulkan::Device &__device;

		std::unique_ptr<Buffer> __pPositionBuffer;
		std::unique_ptr<Buffer> __pColorBuffer;
		std::unique_ptr<Buffer> __pIndexBuffer;
		VkIndexType __indexType{};

		VkBuffer __handles[VERTEX_ATTRIB_LOCATION_MAX]{};
		VkDeviceSize __offsets[VERTEX_ATTRIB_LOCATION_MAX]{};

		Infra::Event<Mesh &> __bufferChangeEvent;
		Infra::Event<Mesh &> __destroyEvent;

		constexpr void __setHandle(const uint32_t attribLocation, Buffer *const pBuffer) noexcept;

		void __setBuffer(
			const VertexAttributeFlagBit attribFlagBit, const uint32_t attribLocation,
			std::unique_ptr<Buffer> &pOldBuffer, std::unique_ptr<Buffer> &&pNewBuffer) noexcept;
	};

	constexpr Infra::EventView<Mesh &> &Mesh::getBufferChangeEvent() noexcept
	{
		return __bufferChangeEvent;
	}

	constexpr Infra::EventView<Mesh &> &Mesh::getDestroyEvent() noexcept
	{
		return __destroyEvent;
	}

	constexpr void Mesh::__setHandle(const uint32_t attribLocation, Buffer *const pBuffer) noexcept
	{
		VkBuffer handle{};
		if (pBuffer)
			handle = pBuffer->getHandle();

		__handles[attribLocation] = handle;
	}
}