#include "Drawcall.h"

namespace HyperFast
{
	Drawcall::Drawcall() noexcept :
		__pAttributeFlagChangeEventListener{ std::make_shared<AttributeFlagChangeEventListener>() }
	{
		__pAttributeFlagChangeEventListener->setCallback(
			std::bind(
				&Drawcall::__onAttributeFlagChange, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
	}

	void Drawcall::addSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		SubmeshGroup &submeshGroup{ __attribFlag2SubmeshGroup[attribFlag] };
		if (submeshGroup.empty())
			__needToUpdate = true;

		submeshGroup[&mesh].emplace(&submesh);
	}

	void Drawcall::removeSubmesh(Submesh &submesh) noexcept
	{
		Mesh &mesh{ submesh.getMesh() };
		const VertexAttributeFlag attribFlag{ mesh.getVertexAttributeFlag() };

		SubmeshGroup &submeshGroup{ __attribFlag2SubmeshGroup[attribFlag] };
		submeshGroup[&mesh].erase(&submesh);

		if (submeshGroup.empty())
			__needToUpdate = true;
	}

	void Drawcall::update() noexcept
	{
		if (!__needToUpdate)
			return;

		__usedAttribFlags.clear();
		for (const auto &[attribFlag, submeshGroup] : __attribFlag2SubmeshGroup)
		{
			if (submeshGroup.empty())
				continue;

			__usedAttribFlags.emplace_back(attribFlag);
		}

		__needToUpdate = false;
		__usedAttributeFlagsChangeEvent.invoke(*this);
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
}