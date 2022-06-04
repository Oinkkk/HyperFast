#include "CommandBufferManager.h"

namespace HyperFast
{
	CommandBufferManager::CommandBufferManager(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		const uint32_t queueFamilyIndex, const size_t numMaxBuffers) noexcept :
		__device{ device }, __deviceProc{ deviceProc },
		__queueFamilyIndex{ queueFamilyIndex }, __numMaxBuffers{ numMaxBuffers }
	{
		__createCommandPool();
	}

	CommandBufferManager::~CommandBufferManager() noexcept
	{
		__destroyCommandPool();
	}

	void CommandBufferManager::getNextBuffers(const size_t numBuffers, std::vector<VkCommandBuffer> &retVal)
	{
		VkCommandBufferAllocateInfo allocInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = __commandPool,
			.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY
		};

		const size_t numCreated{ __commandBuffers.size() };
		if ((numCreated + numBuffers) <= __numMaxBuffers)
		{
			allocInfo.commandBufferCount = uint32_t(numBuffers);
			retVal.resize(numBuffers);

			const VkResult result{ __deviceProc.vkAllocateCommandBuffers(__device, &allocInfo, retVal.data()) };
			if (result != VK_SUCCESS)
				throw std::exception{ "Cannot allocate command buffers." };

			__commandBuffers.insert(__commandBuffers.end(), retVal.begin(), retVal.end());
		}
		else
		{
			if ((__cursor + numBuffers) > __numMaxBuffers)
			{
				__deviceProc.vkResetCommandPool(__device, __commandPool, 0U);
				__cursor = 0ULL;
			}

			const auto from{ __commandBuffers.begin() + __cursor };
			const auto to{ from + numBuffers };

			retVal.clear();
			retVal.insert(retVal.begin(), from, to);
		}

		__cursor += numBuffers;
	}

	void CommandBufferManager::__createCommandPool()
	{
		const VkCommandPoolCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.queueFamilyIndex = __queueFamilyIndex
		};

		__deviceProc.vkCreateCommandPool(__device, &createInfo, nullptr, &__commandPool);
		if (__commandPool)
			return;

		throw std::exception{ "Cannot create the main command pool." };
	}

	void CommandBufferManager::__destroyCommandPool() noexcept
	{
		__deviceProc.vkDestroyCommandPool(__device, __commandPool, nullptr);
		__commandPool = VK_NULL_HANDLE;
	}
}