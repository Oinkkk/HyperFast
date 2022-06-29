#include "Drawcall.h"

namespace HyperFast
{
	Drawcall::Drawcall(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		HyperFast::BufferManager &bufferManager, HyperFast::MemoryManager &memoryManager) noexcept :
		__device{ device }, __deviceProc{ deviceProc },
		__bufferManager{ bufferManager }, __memoryManager{ memoryManager },
		__pAttributeFlagChangeEventListener{ std::make_shared<AttributeFlagChangeEventListener>() },
		__pIndirectBufferUpdateEventListener{ std::make_shared<Infra::EventListener<IndirectBufferBuilder &>>() },
		__pIndirectBufferCreateEventListener{ std::make_shared<Infra::EventListener<IndirectBufferBuilder &>>() }
	{
		__initEventListeners();
	}

	void Drawcall::addSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		IndirectBufferBuilderMap &indirectBufferBuilderMap{ __attribFlag2IndirectBufferMap[attribFlag] };
		IndirectBufferBuilder *pIndirectBufferBuilder{};

		if (indirectBufferBuilderMap.empty())
		{
			mesh.getAttributeFlagChangeEvent() += __pAttributeFlagChangeEventListener;
			__attribFlagsUpdated = true;

			pIndirectBufferBuilder =
				indirectBufferBuilderMap.emplace(
					&mesh, std::make_unique<IndirectBufferBuilder>(
						__device, __deviceProc, __bufferManager, __memoryManager)).first->second.get();

			pIndirectBufferBuilder->getIndirectBufferUpdateEvent() += __pIndirectBufferUpdateEventListener;
			pIndirectBufferBuilder->getIndirectBufferCreateEvent() += __pIndirectBufferCreateEventListener;
		}
		else
			pIndirectBufferBuilder = indirectBufferBuilderMap.at(&mesh).get();

		pIndirectBufferBuilder->addSubmesh(submesh);
	}

	void Drawcall::removeSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		IndirectBufferBuilderMap &indirectBufferBuilderMap{ __attribFlag2IndirectBufferMap[attribFlag] };
		indirectBufferBuilderMap.at(&mesh)->removeSubmesh(submesh);
	}

	void Drawcall::validate()
	{
		if (__attribFlagsUpdated)
		{
			__attribFlags.resize(__attribFlag2IndirectBufferMap.size());

			size_t cursor{};
			for (const auto &[attribFlag, _] : __attribFlag2IndirectBufferMap)
			{
				__attribFlags[cursor] = attribFlag;
				cursor++;
			}

			__attributeFlagsUpdateEvent.invoke(*this);
			__attribFlagsUpdated = false;
		}

		for (const auto &[_, indirectBufferBuilderMap] : __attribFlag2IndirectBufferMap)
		{
			for (const auto &[_, indirectBufferBuilder] : indirectBufferBuilderMap)
				indirectBufferBuilder->validate();
		}

		if (__indirectBufferUpdated)
		{
			__indirectBufferUpdateEvent.invoke(*this);
			__indirectBufferUpdated = false;
		}

		if (__indirectBufferCreated)
		{
			__indirectBufferCreateEvent.invoke(*this);
			__indirectBufferCreated = false;
		}
	}

	void Drawcall::draw(const VertexAttributeFlag attribFlag, VkCommandBuffer commandBuffer) noexcept
	{
		IndirectBufferBuilderMap &indirectBufferBuilderMap{ __attribFlag2IndirectBufferMap[attribFlag] };
		for (const auto &[pMesh, indirectBufferBuilder] : indirectBufferBuilderMap)
		{
			if (indirectBufferBuilder->isEmpty())
				continue;

			pMesh->bind(commandBuffer);
			indirectBufferBuilder->draw(commandBuffer);
		}
	}

	void Drawcall::__initEventListeners() noexcept
	{
		__pAttributeFlagChangeEventListener->setCallback(
			std::bind(
				&Drawcall::__onAttributeFlagChange, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		__pIndirectBufferUpdateEventListener->setCallback(
			std::bind(&Drawcall::__onIndirectBufferUpdate, this, std::placeholders::_1));

		__pIndirectBufferCreateEventListener->setCallback(
			std::bind(&Drawcall::__onIndirectBufferCreate, this, std::placeholders::_1));
	}

	void Drawcall::__onAttributeFlagChange(
		Mesh &mesh, const VertexAttributeFlag oldFlag, VertexAttributeFlag newFlag) noexcept
	{
		IndirectBufferBuilderMap &oldMap{ __attribFlag2IndirectBufferMap[oldFlag] };
		IndirectBufferBuilderMap &newMap{ __attribFlag2IndirectBufferMap[newFlag] };

		newMap.insert(oldMap.extract(&mesh));
	}

	void Drawcall::__onIndirectBufferUpdate(IndirectBufferBuilder &builder) noexcept
	{
		__indirectBufferUpdated = true;
	}

	void Drawcall::__onIndirectBufferCreate(IndirectBufferBuilder &builder) noexcept
	{
		__indirectBufferCreated = true;
	}
}