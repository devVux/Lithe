#pragma once

#include "IResourceCache.hpp"

namespace Lithe {

class ResourceCache: public IResourceCache {

	public:

		[[nodiscard]] MeshData getMesh(MeshID id) const noexcept { 
			return meshes[id];
		}

		[[nodiscard]] bool addMesh(MeshData data) noexcept override {
			mVertexCount += data.position.size();
			mIndexCount += data.indices.size();
			meshes.emplace_back(std::move(data));

			return true;
		}


		[[nodiscard]] virtual std::size_t countVertices() const noexcept { return mVertexCount; }
		[[nodiscard]] virtual std::size_t countIndices() const noexcept { return mIndexCount; }

	private:

		std::size_t mVertexCount = 0;
		std::size_t mIndexCount = 0;

		std::vector<MeshData> meshes;

};

}
