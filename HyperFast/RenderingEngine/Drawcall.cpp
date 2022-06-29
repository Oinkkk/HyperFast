#include "Drawcall.h"

namespace HyperFast
{
	Drawcall::Drawcall() noexcept :
		__pAttributeFlagChangeEventListener{ std::make_shared<AttributeFlagChangeEventListener>() },
		__pSubmeshDestroyEventListener{ std::make_shared<Infra::EventListener<Submesh &>>() }
	{
		__pAttributeFlagChangeEventListener->setCallback(
			std::bind(
				&Drawcall::__onAttributeFlagChange, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		__pSubmeshDestroyEventListener->setCallback(
			std::bind(
				&Drawcall::__onSubmeshDestroy, this,
				std::placeholders::_1));
	}

	void Drawcall::addSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		SubmeshGroup &submeshGroup{ __attribFlag2SubmeshGroup[attribFlag] };
		if (submeshGroup.empty())
		{
			mesh.getAttributeFlagChangeEvent() += __pAttributeFlagChangeEventListener;
			__attribFlagsChanged = true;
		}

		submeshGroup[&mesh].emplace(&submesh);
		__drawcallChanged = true;
	}

	void Drawcall::removeSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		SubmeshGroup &submeshGroup{ __attribFlag2SubmeshGroup[attribFlag] };
		submeshGroup[&mesh].erase(&submesh);
		__drawcallChanged = true;

		if (submeshGroup.empty())
		{
			mesh.getAttributeFlagChangeEvent() -= __pAttributeFlagChangeEventListener;
			__attribFlagsChanged = true;
		}
	}

	void Drawcall::update() noexcept
	{
		if (__attribFlagsChanged)
		{
			__usedAttribFlags.clear();
			for (const auto &[attribFlag, submeshGroup] : __attribFlag2SubmeshGroup)
			{
				if (submeshGroup.empty())
					continue;

				__usedAttribFlags.emplace_back(attribFlag);
			}

			__usedAttributeFlagsChangeEvent.invoke(*this);
			__attribFlagsChanged = false;
		}

		if (__drawcallChanged)
		{
			__drawcallChangeEvent.invoke(*this);
			__drawcallChanged = false;
		}
	}

	void Drawcall::draw(const VertexAttributeFlag attribFlag, VkCommandBuffer commandBuffer) noexcept
	{
		SubmeshGroup &submeshGroup{ __attribFlag2SubmeshGroup[attribFlag] };
		for (const auto &[pMesh, submeshes] : submeshGroup)
		{
			if (submeshes.empty())
				continue;

			pMesh->bind(commandBuffer);
			for (Submesh *const pSubmesh : submeshes)
				pSubmesh->draw(commandBuffer);
		}
	}

	void Drawcall::__onAttributeFlagChange(
		Mesh &mesh, const VertexAttributeFlag oldFlag, VertexAttributeFlag newFlag) noexcept
	{
		SubmeshGroup &oldSubmeshGroup{ __attribFlag2SubmeshGroup[oldFlag] };
		SubmeshGroup &newSubmeshGroup{ __attribFlag2SubmeshGroup[newFlag] };

		newSubmeshGroup.insert(oldSubmeshGroup.extract(&mesh));
	}

	void Drawcall::__onSubmeshDestroy(Submesh &submesh) noexcept
	{
		removeSubmesh(submesh);
	}
}