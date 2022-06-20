#include "Drawcall.h"

namespace HyperFast
{
	void Drawcall::addMesh(const std::shared_ptr<Mesh> &pMesh) noexcept
	{
		const VertexAttributeFlag attribFlag{ pMesh->getVertexAttributeFlag() };

		__usedAttribFlags.emplace(attribFlag);
		__attribFlag2MeshesMap[attribFlag].emplace(pMesh);
	}

	void Drawcall::validate() noexcept
	{
		for (auto mapIter = __attribFlag2MeshesMap.begin(); mapIter != __attribFlag2MeshesMap.end();)
		{
			auto &[attribFlag, meshes] { *mapIter };
			for (auto meshIter = meshes.begin(); meshIter != meshes.end();)
			{
				if (meshIter->expired())
				{
					meshIter = meshes.erase(meshIter);
					continue;
				}

				meshIter++;
			}

			if (meshes.empty())
			{
				__usedAttribFlags.erase(attribFlag);
				mapIter = __attribFlag2MeshesMap.erase(mapIter);
				continue;
			}

			mapIter++;
		}
	}

	void Drawcall::bind(const VertexAttributeFlag attribFlag, const VkCommandBuffer commandBuffer) noexcept
	{
		for (const std::weak_ptr<Mesh> &pMesh : __attribFlag2MeshesMap[attribFlag])
			pMesh.lock()->bind(commandBuffer);
	}
}