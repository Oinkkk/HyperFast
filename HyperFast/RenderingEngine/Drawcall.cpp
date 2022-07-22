﻿#include "Drawcall.h"
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

		Mesh2ResourceMap &mesh2ResourceMap{ __attribFlag2MeshResourceMap[attribFlag] };
		if (mesh2ResourceMap.empty())
			__attribFlagsUpdated = true;

		auto &[pCommandBufferManager, pIndirectBufferBuilder]{ mesh2ResourceMap[&mesh] };

		// 처음 본 메시
		if (!pIndirectBufferBuilder)
		{
			pCommandBufferManager = std::make_unique<CommandBufferManager>(__device, __queueFamilyIndex);
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

		Mesh2ResourceMap &mesh2ResourceMap{ __attribFlag2MeshResourceMap[attribFlag] };
		auto &[pCommandBufferManager, pIndirectBufferBuilder] { mesh2ResourceMap[&mesh] };

		submesh.getDestroyEvent() -= __pSubmeshDestroyEventListener;
		pIndirectBufferBuilder->removeSubmesh(submesh);
	}

	void Drawcall::update()
	{
		if (__attribFlagsUpdated)
		{
			__attribFlags.resize(__attribFlag2MeshResourceMap.size());

			size_t cursor{};
			for (const auto &[attribFlag, _] : __attribFlag2MeshResourceMap)
			{
				__attribFlags[cursor] = attribFlag;
				cursor++;
			}

			__needToUpdatePipelineDependencies = true;
			__attribFlagsUpdated = false;
		}

		for (const auto &[_, mesh2ResourceMap] : __attribFlag2MeshResourceMap)
		{
			for (const auto &[_, perMeshResource] : mesh2ResourceMap)
			{
				auto &[_, pIndirectBufferBuilder] { perMeshResource };
				pIndirectBufferBuilder->update();
			}
		}

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
		Mesh2ResourceMap &mesh2ResourceMap{ __attribFlag2MeshResourceMap[attribFlag] };
		for (const auto &[pMesh, perMeshResource] : mesh2ResourceMap)
		{
			auto &[_, pIndirectBufferBuilder] { perMeshResource };

			pMesh->bind(commandBuffer);
			pIndirectBufferBuilder->draw(commandBuffer);
		}
	}

	void Drawcall::addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		for (const auto &[_, mesh2ResourceMap] : __attribFlag2MeshResourceMap)
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

	void Drawcall::__onMeshAttributeFlagChange(
		Mesh &mesh, const VertexAttributeFlag oldFlag, VertexAttributeFlag newFlag) noexcept
	{
		Mesh2ResourceMap &oldMap{ __attribFlag2MeshResourceMap[oldFlag] };
		Mesh2ResourceMap &newMap{ __attribFlag2MeshResourceMap[newFlag] };

		newMap.insert(oldMap.extract(&mesh));
	}

	void Drawcall::__onMeshBufferChange(Mesh &mesh) noexcept
	{
		__needToUpdateMainCommands = true;
	}

	void Drawcall::__onMeshDestroy(Mesh &mesh) noexcept
	{
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		Mesh2ResourceMap &mesh2ResourceMap{ __attribFlag2MeshResourceMap[attribFlag] };
		mesh2ResourceMap.erase(&mesh);
	}

	void Drawcall::__onSubmeshDestroy(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		Mesh2ResourceMap &mesh2ResourceMap{ __attribFlag2MeshResourceMap[attribFlag] };
		auto &[_, pIndirectBufferBuilder]{ mesh2ResourceMap[&mesh] };

		pIndirectBufferBuilder->removeSubmesh(submesh);
	}

	void Drawcall::__onIndirectBufferUpdate(IndirectBufferBuilder &builder) noexcept
	{
		__needToRender = true;
	}

	void Drawcall::__onIndirectBufferCreate(IndirectBufferBuilder &builder) noexcept
	{
		__needToUpdateMainCommands = true;
	}
}