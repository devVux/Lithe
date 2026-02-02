#pragma once

#include "DTO.hpp"

namespace Lithe {

class IResourceCache {

	public:
		
		virtual ~IResourceCache() = default;

		[[nodiscard]] virtual MeshData getMesh(MeshID) const noexcept = 0;
		[[nodiscard]] virtual bool addMesh(MeshData data) noexcept = 0;

		[[nodiscard]] virtual std::size_t countVertices() const noexcept = 0;
		[[nodiscard]] virtual std::size_t countIndices() const noexcept = 0;

};

}
