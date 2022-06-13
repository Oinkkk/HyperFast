#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/VulkanLoader.h"

namespace HyperFast
{
	class BufferManager final : public Infra::Unique
	{
	public:
		class BufferImpl final : public Infra::Unique
		{
		public:

		};

		[[nodiscard]]
		BufferImpl *create(const VkDeviceSize memSize, const VkBufferUsageFlags usage);

	private:
	};
}