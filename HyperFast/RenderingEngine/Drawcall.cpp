#include "Drawcall.h"
#include <iostream>

namespace HyperFast
{
	Drawcall::Drawcall(
		Vulkan::Device &device, BufferManager &bufferManager,
		MemoryManager &memoryManager) noexcept :
		__device{ device }, __bufferManager{ bufferManager },
		__memoryManager{ memoryManager }
	{
		__initEventListeners();
	}

	void Drawcall::addSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		IndirectBufferBuilderMap &indirectBufferBuilderMap{ __attribFlag2IndirectBufferMap[attribFlag] };
		if (indirectBufferBuilderMap.empty())
			__attribFlagsUpdated = true;

		auto &pIndirectBufferBuilder{ indirectBufferBuilderMap[&mesh] };
		if (!pIndirectBufferBuilder)
		{
			pIndirectBufferBuilder =
				std::make_unique<IndirectBufferBuilder>(__device, __bufferManager, __memoryManager);

			pIndirectBufferBuilder->getIndirectBufferUpdateEvent() += __pIndirectBufferUpdateEventListener;
			pIndirectBufferBuilder->getIndirectBufferCreateEvent() += __pIndirectBufferCreateEventListener;
		}

		if (pIndirectBufferBuilder->isEmpty())
		{
			mesh.getAttributeFlagChangeEvent() += __pMeshAttributeFlagChangeEventListener;
			mesh.getBufferChangeEvent() += __pMeshBufferChangeEventListener;
			__meshBufferChanged = true;
		}

		pIndirectBufferBuilder->addSubmesh(submesh);
	}

	void Drawcall::removeSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		IndirectBufferBuilderMap &indirectBufferBuilderMap{ __attribFlag2IndirectBufferMap[attribFlag] };
		auto &pIndirectBufferBuilder{ indirectBufferBuilderMap[&mesh] };

		pIndirectBufferBuilder->removeSubmesh(submesh);

		if (pIndirectBufferBuilder->isEmpty())
		{
			mesh.getAttributeFlagChangeEvent() -= __pMeshAttributeFlagChangeEventListener;
			mesh.getBufferChangeEvent() -= __pMeshBufferChangeEventListener;
			__meshBufferChanged = true;
		}
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

		if (__meshBufferChanged)
		{
			__meshBufferChangeEvent.invoke(*this);
			__meshBufferChanged = false;
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

	void Drawcall::render(const VertexAttributeFlag attribFlag, Vulkan::CommandBuffer &commandBuffer) noexcept
	{
		IndirectBufferBuilderMap &indirectBufferBuilderMap{ __attribFlag2IndirectBufferMap[attribFlag] };
		for (const auto &[pMesh, pIndirectBufferBuilder] : indirectBufferBuilderMap)
		{
			pMesh->bind(commandBuffer);
			pIndirectBufferBuilder->render(commandBuffer);
		}
	}

	void Drawcall::addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		for (const auto &[_, indirectBufferBuilderMap] : __attribFlag2IndirectBufferMap)
		{
			for (const auto &[pMesh, _] : indirectBufferBuilderMap)
				pMesh->addSemaphoreDependency(pDependency);
		}
	}

	void Drawcall::__initEventListeners() noexcept
	{
		__pMeshAttributeFlagChangeEventListener =
			MeshAttributeFlagChangeEventListener::bind(
				&Drawcall::__onMeshAttributeFlagChange, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

		__pMeshBufferChangeEventListener =
			Infra::EventListener<Mesh &>::bind(
				&Drawcall::__onMeshBufferChange, this, std::placeholders::_1);

		__pIndirectBufferUpdateEventListener =
			Infra::EventListener<IndirectBufferBuilder &>::bind(
				&Drawcall::__onIndirectBufferUpdate, this, std::placeholders::_1);

		__pIndirectBufferCreateEventListener =
			Infra::EventListener<IndirectBufferBuilder &>::bind(
				&Drawcall::__onIndirectBufferCreate, this, std::placeholders::_1);
	}

	void Drawcall::__onMeshAttributeFlagChange(
		Mesh &mesh, const VertexAttributeFlag oldFlag, VertexAttributeFlag newFlag) noexcept
	{
		IndirectBufferBuilderMap &oldMap{ __attribFlag2IndirectBufferMap[oldFlag] };
		IndirectBufferBuilderMap &newMap{ __attribFlag2IndirectBufferMap[newFlag] };

		newMap.insert(oldMap.extract(&mesh));
	}

	void Drawcall::__onMeshBufferChange(Mesh &mesh) noexcept
	{
		__meshBufferChanged = true;
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