#pragma once

#include "DTO.hpp"

namespace Lithe {

class IResourceCache {

	public:
		
		virtual ~IResourceCache() = default;

		virtual MeshData getMesh(MeshID) = 0;

};

}
