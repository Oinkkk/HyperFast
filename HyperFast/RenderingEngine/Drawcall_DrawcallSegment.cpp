#include "Drawcall.h"

namespace HyperFast
{
	Drawcall::DrawcallSegment::DrawcallSegment(
		const size_t segmentIndex, Vulkan::Device &device, const uint32_t queueFamilyIndex,
		BufferManager &bufferManager, MemoryManager &memoryManager) noexcept :
		__segmentIndex{ segmentIndex }, __device { device }, __queueFamilyIndex{queueFamilyIndex},
		__bufferManager{ bufferManager }, __memoryManager{ memoryManager }
	{
		__initEventListeners();
	}

	void Drawcall::DrawcallSegment::addSubmesh(Submesh &submesh) noexcept
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

	void Drawcall::DrawcallSegment::removeSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const std::unique_ptr<IndirectBufferBuilder> &pIndirectBufferBuilder{ __mesh2BuilderMap[&mesh] };

		submesh.getDestroyEvent() -= __pSubmeshDestroyEventListener;
		pIndirectBufferBuilder->removeSubmesh(submesh);
	}

	void Drawcall::DrawcallSegment::update()
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

	void Drawcall::DrawcallSegment::draw(Vulkan::CommandBuffer &commandBuffer) noexcept
	{
		for (const auto &[pMesh, pIndirectBufferBuilder] : __mesh2BuilderMap)
		{
			pMesh->bind(commandBuffer);
			pIndirectBufferBuilder->draw(commandBuffer);
		}
	}

	void Drawcall::DrawcallSegment::addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		for (const auto &[pMesh, _] : __mesh2BuilderMap)
			pMesh->addSemaphoreDependency(pDependency);
	}

	void Drawcall::DrawcallSegment::__initEventListeners() noexcept
	{
		__pMeshBufferChangeEventListener =
			Infra::EventListener<Mesh &>::bind(
				&Drawcall::DrawcallSegment::__onMeshBufferChange, this, std::placeholders::_1);

		__pMeshDestroyEventListener =
			Infra::EventListener<Mesh &>::bind(
				&Drawcall::DrawcallSegment::__onMeshDestroy, this, std::placeholders::_1);

		__pSubmeshDestroyEventListener =
			Infra::EventListener<Submesh &>::bind(
				&Drawcall::DrawcallSegment::__onSubmeshDestroy, this, std::placeholders::_1);

		__pIndirectBufferUpdateEventListener =
			Infra::EventListener<IndirectBufferBuilder &>::bind(
				&Drawcall::DrawcallSegment::__onIndirectBufferUpdate, this, std::placeholders::_1);

		__pIndirectBufferCreateEventListener =
			Infra::EventListener<IndirectBufferBuilder &>::bind(
				&Drawcall::DrawcallSegment::__onIndirectBufferCreate, this, std::placeholders::_1);
	}

	void Drawcall::DrawcallSegment::__updateIndirectBufferBuilders() noexcept
	{
		for (const auto &[_, pIndirectBufferBuilder] : __mesh2BuilderMap)
			pIndirectBufferBuilder->update();
	}

	void Drawcall::DrawcallSegment::__onMeshBufferChange(Mesh &mesh) noexcept
	{
		__meshBufferChanged = true;
	}

	void Drawcall::DrawcallSegment::__onMeshDestroy(Mesh &mesh) noexcept
	{
		__mesh2BuilderMap.erase(&mesh);
	}

	void Drawcall::DrawcallSegment::__onSubmeshDestroy(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const std::unique_ptr<IndirectBufferBuilder> &pIndirectBufferBuilder{ __mesh2BuilderMap[&mesh] };

		pIndirectBufferBuilder->removeSubmesh(submesh);
	}

	void Drawcall::DrawcallSegment::__onIndirectBufferUpdate(IndirectBufferBuilder &builder) noexcept
	{
		__indirectBufferUpdated = true;
	}

	void Drawcall::DrawcallSegment::__onIndirectBufferCreate(IndirectBufferBuilder &builder) noexcept
	{
		__indirectBufferCreated = true;
	}
}