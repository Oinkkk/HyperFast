#include "Drawcall.h"
#include <iostream>

namespace HyperFast
{
	Drawcall::Drawcall(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		BufferManager &bufferManager, MemoryManager &memoryManager) noexcept :
		__device{ device }, __queueFamilyIndex{ queueFamilyIndex },
		__bufferManager{ bufferManager }, __memoryManager{ memoryManager }
	{
		__initEventListeners();
	}

	void Drawcall::addSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		std::unique_ptr<IndirectBufferBuilder> &pIndirectBufferBuilder{ __mesh2BuilderMap[&mesh] };

		if (!pIndirectBufferBuilder)
		{
			pIndirectBufferBuilder =
				std::make_unique<IndirectBufferBuilder>(__device, __bufferManager, __memoryManager);

			pIndirectBufferBuilder->getIndirectBufferUpdateEvent() += __pIndirectBufferUpdateEventListener;
			pIndirectBufferBuilder->getIndirectBufferCreateEvent() += __pIndirectBufferCreateEventListener;

			mesh.getBufferChangeEvent() += __pMeshBufferChangeEventListener;
			mesh.getDestroyEvent() += __pMeshDestroyEventListener;
		}

		pIndirectBufferBuilder->addSubmesh(submesh);
		submesh.getDestroyEvent() += __pSubmeshDestroyEventListener;
	}

	void Drawcall::removeSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const std::unique_ptr<IndirectBufferBuilder> &pIndirectBufferBuilder{ __mesh2BuilderMap[&mesh] };

		submesh.getDestroyEvent() -= __pSubmeshDestroyEventListener;
		pIndirectBufferBuilder->removeSubmesh(submesh);
	}

	void Drawcall::update()
	{
		__updateIndirectBufferBuilders();

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

	void Drawcall::draw(Vulkan::CommandBuffer &commandBuffer) noexcept
	{
		for (const auto &[pMesh, pIndirectBufferBuilder] : __mesh2BuilderMap)
		{
			pMesh->bind(commandBuffer);
			pIndirectBufferBuilder->draw(commandBuffer);
		}
	}

	void Drawcall::addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		for (const auto &[pMesh, _] : __mesh2BuilderMap)
			pMesh->addSemaphoreDependency(pDependency);
	}

	void Drawcall::__initEventListeners() noexcept
	{
		__pMeshBufferChangeEventListener =
			Infra::EventListener<Mesh &>::bind(
				&Drawcall::__onMeshBufferChange, this, std::placeholders::_1);

		__pMeshDestroyEventListener =
			Infra::EventListener<Mesh &>::bind(
				&Drawcall::__onMeshDestroy, this, std::placeholders::_1);

		__pSubmeshDestroyEventListener =
			Infra::EventListener<Submesh &>::bind(
				&Drawcall::__onSubmeshDestroy, this, std::placeholders::_1);

		__pIndirectBufferUpdateEventListener =
			Infra::EventListener<IndirectBufferBuilder &>::bind(
				&Drawcall::__onIndirectBufferUpdate, this, std::placeholders::_1);

		__pIndirectBufferCreateEventListener =
			Infra::EventListener<IndirectBufferBuilder &>::bind(
				&Drawcall::__onIndirectBufferCreate, this, std::placeholders::_1);
	}

	void Drawcall::__updateIndirectBufferBuilders() noexcept
	{
		for (const auto &[_, pIndirectBufferBuilder] : __mesh2BuilderMap)
			pIndirectBufferBuilder->update();
	}

	void Drawcall::__onMeshBufferChange(Mesh &mesh) noexcept
	{
		__meshBufferChanged = true;
	}

	void Drawcall::__onMeshDestroy(Mesh &mesh) noexcept
	{
		__mesh2BuilderMap.erase(&mesh);
	}

	void Drawcall::__onSubmeshDestroy(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const std::unique_ptr<IndirectBufferBuilder> &pIndirectBufferBuilder{ __mesh2BuilderMap[&mesh] };

		pIndirectBufferBuilder->removeSubmesh(submesh);
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