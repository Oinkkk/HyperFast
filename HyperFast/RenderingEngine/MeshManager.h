#pragma once

#include <vector>
#include "DeviceBuffer.h"
#include "VertexAttribute.h"
#include <unordered_map>
#include <mutex>

namespace HyperFast
{
	class MeshBufferInfo
	{
	public:
		uint32_t location{};
		size_t memSize{};
		const void *pData{};
	};

	class MeshBufferBuildInfo
	{
	public:
		VertexAttributeFlagBit attributeType{};
		MeshBufferInfo bufferInfo{};
	};

	class MeshBuildParam
	{
	public:
		std::vector<MeshBufferBuildInfo> bufferBuildInfos;
	};

	class MeshUpdateParam
	{
	public:
		std::vector<MeshBufferInfo> bufferInfos;
	};

	class MeshManager final : public Infra::Unique
	{
	public:
		class MeshImpl final : public Infra::Unique
		{
		public:
			MeshImpl(DeviceBufferManager &deviceBufferManager) noexcept;
			~MeshImpl() noexcept = default;

			void setBuildParam(const MeshBuildParam &buildParam) noexcept;
			void build();

			void setUpdateParam(const MeshUpdateParam &updateParam) noexcept;
			void update();

			[[nodiscard]]
			constexpr const std::vector<VkVertexInputBindingDescription> &getBindingDescriptions() const noexcept;

			[[nodiscard]]
			constexpr const std::vector<VkVertexInputAttributeDescription> &getAttributeDescriptions() const noexcept;

			[[nodiscard]]
			constexpr VertexAttributeFlag getVertexAttributeFlag() const noexcept;

		private:
			DeviceBufferManager &__deviceBufferManager;

			std::mutex __buildParamMutex;
			MeshBuildParam __buildParam;

			std::mutex __updateParamMutex;
			MeshUpdateParam __updateParam;

			std::vector<VkVertexInputBindingDescription> __bindingDescs;
			std::vector<VkVertexInputAttributeDescription> __attribDescs;
			VertexAttributeFlag __attributeFlag{};

			std::unordered_map<size_t, std::unique_ptr<DeviceBuffer>> __deviceBufferMap;

			void __buildMetadata() noexcept;
			void __buildBuffers();
		};

		MeshManager(DeviceBufferManager &deviceBufferManager) noexcept;
		~MeshManager() noexcept = default;

		[[nodiscard]]
		std::unique_ptr<MeshImpl> create() noexcept;

	private:
		DeviceBufferManager &__deviceBufferManager;
	};

	constexpr const std::vector<VkVertexInputBindingDescription> &
		MeshManager::MeshImpl::getBindingDescriptions() const noexcept
	{
		return __bindingDescs;
	}

	constexpr const std::vector<VkVertexInputAttributeDescription> &
		MeshManager::MeshImpl::getAttributeDescriptions() const noexcept
	{
		return __attribDescs;
	}

	constexpr VertexAttributeFlag MeshManager::MeshImpl::getVertexAttributeFlag() const noexcept
	{
		return __attributeFlag;
	}
}