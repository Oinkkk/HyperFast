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
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		Mesh2BuilderMap &mesh2BuilderMap{ __attribFlag2MeshBuilderMap[attribFlag] };
		if (mesh2BuilderMap.empty())
			__attribFlagsUpdated = true;

		auto &pIndirectBufferBuilder{ mesh2BuilderMap[&mesh] };

		// 처음 본 메시
		if (!pIndirectBufferBuilder)
		{
			pIndirectBufferBuilder =
				std::make_unique<IndirectBufferBuilder>(__device, __bufferManager, __memoryManager);

			pIndirectBufferBuilder->getIndirectBufferUpdateEvent() += __pIndirectBufferUpdateEventListener;
			pIndirectBufferBuilder->getIndirectBufferCreateEvent() += __pIndirectBufferCreateEventListener;

			mesh.getAttributeFlagChangeEvent() += __pMeshAttributeFlagChangeEventListener;
			mesh.getBufferChangeEvent() += __pMeshBufferChangeEventListener;
			mesh.getDestroyEvent() += __pMeshDestroyEventListener;

			__needToUpdateMainCommands = true;
		}

		pIndirectBufferBuilder->addSubmesh(submesh);
		submesh.getDestroyEvent() += __pSubmeshDestroyEventListener;
	}

	void Drawcall::removeSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		Mesh2BuilderMap &mesh2BuilderMap{ __attribFlag2MeshBuilderMap[attribFlag] };
		auto &pIndirectBufferBuilder{ mesh2BuilderMap[&mesh] };

		submesh.getDestroyEvent() -= __pSubmeshDestroyEventListener;
		pIndirectBufferBuilder->removeSubmesh(submesh);
	}

	void Drawcall::update()
	{
		if (__attribFlagsUpdated)
		{
			__updateAttributeFlagVector();
			__needToUpdatePipelineDependencies = true;
			__attribFlagsUpdated = false;
		}

		__updateIndirectBufferBuilders();

		if (__needToUpdatePipelineDependencies)
		{
			__needToUpdatePipelineDependenciesEvent.invoke(*this);
			__needToUpdatePipelineDependencies = false;
		}

		if (__needToUpdateMainCommands)
		{
			__needToUpdateMainCommandsEvent.invoke(*this);
			__needToUpdateMainCommands = false;
		}

		if (__needToRender)
		{
			__needToRenderEvent.invoke(*this);
			__needToRender = false;
		}
	}

	void Drawcall::draw(const VertexAttributeFlag attribFlag, Vulkan::CommandBuffer &commandBuffer) noexcept
	{
		Mesh2BuilderMap &mesh2BuilderMap{ __attribFlag2MeshBuilderMap[attribFlag] };
		for (const auto &[pMesh, pIndirectBufferBuilder] : mesh2BuilderMap)
		{
			pMesh->bind(commandBuffer);
			pIndirectBufferBuilder->draw(commandBuffer);
		}
	}

	void Drawcall::addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		for (const auto &[_, mesh2ResourceMap] : __attribFlag2MeshBuilderMap)
		{
			for (const auto &[pMesh, _] : mesh2ResourceMap)
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

	void Drawcall::__updateAttributeFlagVector() noexcept
	{
		__attribFlags.clear();

		for (const auto &[attribFlag, _] : __attribFlag2MeshBuilderMap)
			__attribFlags.emplace_back(attribFlag);
	}

	void Drawcall::__updateIndirectBufferBuilders() noexcept
	{
		for (const auto &[_, mesh2BuilderMap] : __attribFlag2MeshBuilderMap)
		{
			for (const auto &[_, pIndirectBufferBuilder] : mesh2BuilderMap)
				pIndirectBufferBuilder->update();
		}
	}

	void Drawcall::__onMeshAttributeFlagChange(
		Mesh &mesh, const VertexAttributeFlag oldFlag, VertexAttributeFlag newFlag) noexcept
	{
		Mesh2BuilderMap &oldMap{ __attribFlag2MeshBuilderMap[oldFlag] };
		Mesh2BuilderMap &newMap{ __attribFlag2MeshBuilderMap[newFlag] };

		newMap.insert(oldMap.extract(&mesh));
	}

	void Drawcall::__onMeshBufferChange(Mesh &mesh) noexcept
	{
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		Mesh2BuilderMap &mesh2BuilderMap{ __attribFlag2MeshBuilderMap[attribFlag] };
		auto &pIndirectBufferBuilder{ mesh2BuilderMap[&mesh] };

		__dirtyBuilders.emplace(pIndirectBufferBuilder.get());
	}

	void Drawcall::__onMeshDestroy(Mesh &mesh) noexcept
	{
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		Mesh2BuilderMap &mesh2ResourceMap{ __attribFlag2MeshBuilderMap[attribFlag] };
		mesh2ResourceMap.erase(&mesh);
	}

	void Drawcall::__onSubmeshDestroy(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		Mesh2BuilderMap &mesh2BuilderMap{ __attribFlag2MeshBuilderMap[attribFlag] };
		auto &pIndirectBufferBuilder{ mesh2BuilderMap[&mesh] };

		pIndirectBufferBuilder->removeSubmesh(submesh);
	}

	void Drawcall::__onIndirectBufferUpdate(IndirectBufferBuilder &builder) noexcept
	{
		__needToRender = true;
	}

	void Drawcall::__onIndirectBufferCreate(IndirectBufferBuilder &builder) noexcept
	{
		__dirtyBuilders.emplace(&builder);
	}
}