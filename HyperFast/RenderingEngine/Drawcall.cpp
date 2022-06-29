#include "Drawcall.h"

namespace HyperFast
{
	Drawcall::Drawcall() noexcept :
		__pAttributeFlagChangeEventListener{ std::make_shared<AttributeFlagChangeEventListener>() },
		__pSubmeshVisibleChangeEventListener{ std::make_shared<Infra::EventListener<Submesh &>>() },
		__pSubmeshDestroyEventListener{ std::make_shared<Infra::EventListener<Submesh &>>() }
	{
		__pAttributeFlagChangeEventListener->setCallback(
			std::bind(
				&Drawcall::__onAttributeFlagChange, this,
				std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

		__pSubmeshVisibleChangeEventListener->setCallback(
			std::bind(
				&Drawcall::__onSubmeshVisibleChange, this,
				std::placeholders::_1));

		__pSubmeshDestroyEventListener->setCallback(
			std::bind(
				&Drawcall::__onSubmeshDestroy, this,
				std::placeholders::_1));
	}

	void Drawcall::addSubmesh(Submesh &submesh) noexcept
	{
		submesh.getVisibleChangeEvent() += __pSubmeshVisibleChangeEventListener;
		submesh.getDestroyEvent() += __pSubmeshDestroyEventListener;

		if (submesh.isVisible())
			__registerSubmesh(submesh);
	}

	void Drawcall::removeSubmesh(Submesh &submesh) noexcept
	{
		submesh.getVisibleChangeEvent() -= __pSubmeshVisibleChangeEventListener;
		submesh.getDestroyEvent() -= __pSubmeshDestroyEventListener;

		if (submesh.isVisible())
			__unregisterSubmesh(submesh);
	}

	void Drawcall::draw() noexcept
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

	void Drawcall::__registerSubmesh(Submesh &submesh) noexcept
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

	void Drawcall::__unregisterSubmesh(Submesh &submesh) noexcept
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

	void Drawcall::__onAttributeFlagChange(
		Mesh &mesh, const VertexAttributeFlag oldFlag, VertexAttributeFlag newFlag) noexcept
	{
		SubmeshGroup &oldSubmeshGroup{ __attribFlag2SubmeshGroup[oldFlag] };
		SubmeshGroup &newSubmeshGroup{ __attribFlag2SubmeshGroup[newFlag] };

		newSubmeshGroup.insert(oldSubmeshGroup.extract(&mesh));
	}

	void Drawcall::__onSubmeshVisibleChange(Submesh &submesh) noexcept
	{
		if (submesh.isVisible())
			__registerSubmesh(submesh);
		else
			__unregisterSubmesh(submesh);
	}

	void Drawcall::__onSubmeshDestroy(Submesh &submesh) noexcept
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
}