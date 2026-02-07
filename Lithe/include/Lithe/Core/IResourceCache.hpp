#pragma once

#include "RenderData.hpp"

#include <optional>

namespace Lithe {

class IResourceCache {

	public:
		
		virtual ~IResourceCache() = default;

		[[nodiscard]] virtual MeshData getMesh(MeshID) const noexcept = 0;
		[[nodiscard]] virtual std::optional<MeshID> addMesh(MeshData data) noexcept = 0;

		[[nodiscard]] virtual MaterialData getMaterial(MaterialID) const noexcept = 0;
		[[nodiscard]] virtual std::optional<MaterialID> addMaterial(MaterialData) noexcept = 0;

		[[nodiscard]] virtual TextureData getTexture(TextureID) const noexcept = 0;
		[[nodiscard]] virtual std::optional<TextureID> addTexture(TextureData) noexcept = 0;

		[[nodiscard]] virtual std::size_t vertexCount() const noexcept = 0;
		[[nodiscard]] virtual std::size_t indexCount() const noexcept = 0;
		[[nodiscard]] virtual std::size_t materialCount() const noexcept = 0;
		[[nodiscard]] virtual std::size_t textureCount() const noexcept = 0;

		[[nodiscard]] virtual Size largestTexture() const noexcept = 0;


		// TODO: revisit this
		// We could do many things as returning IDs and querying the cache directly or
		// use the visitor pattern for more complex iteration logic
		[[nodiscard]] virtual std::vector<std::pair<MeshID, MeshData>> meshes() const noexcept = 0;
		[[nodiscard]] virtual std::vector<std::pair<MaterialID, MaterialData>> materials() const noexcept = 0;
		[[nodiscard]] virtual std::vector<std::pair<TextureID, TextureData>> textures() const noexcept = 0;

};

}
