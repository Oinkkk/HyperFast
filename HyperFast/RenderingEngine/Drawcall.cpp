#include "Drawcall.h"
#include <thread>

namespace HyperFast
{
	Drawcall::Drawcall(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		BufferManager &bufferManager, MemoryManager &memoryManager, LifeCycle &lifeCycle) noexcept
	{
		__initEventListeners();
		__createSegments(device, queueFamilyIndex, bufferManager, memoryManager);

		lifeCycle.getSignalEvent(LifeCycleType::DATA_UPDATE) += __pUpdateEventListener;
	}

	void Drawcall::addSubmesh(Submesh &submesh) noexcept
	{
		Mesh *const pMesh{ &(submesh.getMesh()) };
		DrawcallSegment *&pSegment{ __mesh2SegmentMap[pMesh] };

		if (!pSegment)
		{
			pSegment = __getCurrentSegment();
			__advanceSegment();

			pMesh->getDestroyEvent() += __pMeshDestroyEventListener;
		}

		pSegment->addSubmesh(submesh);
	}

	void Drawcall::removeSubmesh(Submesh &submesh) noexcept
	{
		Mesh *const pMesh{ &(submesh.getMesh()) };
		DrawcallSegment *const pSegment{ __mesh2SegmentMap[pMesh] };

		pSegment->removeSubmesh(submesh);
	}

	void Drawcall::draw(const size_t segmentIndex, Vulkan::CommandBuffer &commandBuffer) noexcept
	{
		__segments[segmentIndex]->draw(commandBuffer);
	}

	void Drawcall::__initEventListeners() noexcept
	{
		__pMeshDestroyEventListener =
			Infra::EventListener<Mesh &>::bind(
				&Drawcall::__onMeshDestroy, this, std::placeholders::_1);

		__pSegmentMeshBufferChangeEventListener =
			Infra::EventListener<DrawcallSegment &>::bind(
				&Drawcall::__onSegmentMeshBufferChange, this, std::placeholders::_1);

		__pSegmentIndirectBufferUpdateEventListener =
			Infra::EventListener<DrawcallSegment &>::bind(
				&Drawcall::__onSegmentIndirectBufferUpdate, this, std::placeholders::_1);

		__pSegmentIndirectBufferCreateEventListener =
			Infra::EventListener<DrawcallSegment &>::bind(
				&Drawcall::__onSegmentIndirectBufferCreate, this, std::placeholders::_1);

		__pUpdateEventListener = Infra::EventListener<>::bind(&Drawcall::__onUpdate, this);
	}

	void Drawcall::__createSegments(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		BufferManager &bufferManager, MemoryManager &memoryManager) noexcept
	{
		const size_t numSegments{ std::thread::hardware_concurrency() };
		for (size_t segmentIter = 0ULL; segmentIter < numSegments; segmentIter++)
		{
			DrawcallSegment *const pSegment
			{
				__segments.emplace_back(
					std::make_unique<DrawcallSegment>(
						segmentIter, device, queueFamilyIndex,
						bufferManager, memoryManager)).get()
			};

			pSegment->getMeshBufferChangeEvent() += __pSegmentMeshBufferChangeEventListener;
			pSegment->getIndirectBufferUpdateEvent() += __pSegmentIndirectBufferUpdateEventListener;
			pSegment->getIndirectBufferCreateEvent() += __pSegmentIndirectBufferCreateEventListener;
		}
	}

	Drawcall::DrawcallSegment *Drawcall::__getCurrentSegment() noexcept
	{
		return __segments[__segmentCursor].get();
	}

	void Drawcall::__onMeshDestroy(Mesh &mesh) noexcept
	{
		__mesh2SegmentMap.erase(&mesh);
	}

	void Drawcall::__onSegmentMeshBufferChange(DrawcallSegment &segment) noexcept
	{
		__segmentMeshBufferChangeEvent.invoke(*this, segment.getSegmentIndex());
	}

	void Drawcall::__onSegmentIndirectBufferUpdate(DrawcallSegment &segment) noexcept
	{
		__segmentIndirectBufferUpdateEvent.invoke(*this, segment.getSegmentIndex());
	}

	void Drawcall::__onSegmentIndirectBufferCreate(DrawcallSegment &segment) noexcept
	{
		__segmentIndirectBufferCreateEvent.invoke(*this, segment.getSegmentIndex());
	}

	void Drawcall::__onUpdate()
	{
		for (const auto &pSegment : __segments)
			pSegment->update();
	}
}