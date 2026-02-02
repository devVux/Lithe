#pragma once

#include "DTO.hpp"

namespace Lithe {

class IResourceCache {

	public:
		
		virtual ~IResourceCache() = default;

		[[nodiscard]] virtual MeshData getMesh(MeshID) const noexcept = 0;

};

}
