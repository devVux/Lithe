#pragma once

#include "IResourceCache.hpp"

namespace Lithe {

class ResourceCache: public IResourceCache {

public:

	[[nodiscard]] MeshData getMesh(MeshID id) const noexcept override { return mMeshes[static_cast<std::size_t>(id)]; }

	[[nodiscard]] std::optional<MeshID> addMesh(MeshData data) noexcept override {
		mVertexCount += data.positions.size();
		mIndexCount	 += data.indices.size();
		mMeshes.emplace_back(std::move(data));

		return static_cast<MeshID>(mMeshes.size() - 1);
	}

	[[nodiscard]] MaterialData getMaterial(MaterialID id) const noexcept override {
		return mMaterials[static_cast<std::size_t>(id)];
	}

	[[nodiscard]] std::optional<MaterialID> addMaterial(MaterialData data) noexcept override {
		mMaterials.emplace_back(data);

		return static_cast<MaterialID>(mMaterials.size() - 1);
	}

	[[nodiscard]] TextureData getTexture(TextureID id) const noexcept override {
		return mTextures[static_cast<std::size_t>(id)];
	}

	[[nodiscard]] std::optional<TextureID> addTexture(TextureData data) noexcept override {
		if (data.width * data.height > mLargestTexture.width * mLargestTexture.height)
			mLargestTexture = {data.width, data.height};

		mTextures.emplace_back(std::move(data));

		return static_cast<TextureID>(mTextures.size() - 1);
	}

	[[nodiscard]] std::size_t vertexCount() const noexcept override { return mVertexCount; }

	[[nodiscard]] std::size_t indexCount() const noexcept override { return mIndexCount; }

	[[nodiscard]] std::size_t materialCount() const noexcept override { return mMaterials.size(); }

	[[nodiscard]] std::size_t textureCount() const noexcept override { return mTextures.size(); }

	[[nodiscard]] Size2 largestTexture() const noexcept override { return mLargestTexture; }

	[[nodiscard]] std::vector<std::pair<MeshID, MeshData>> meshes() const noexcept override {
		std::vector<std::pair<MeshID, MeshData>> result;
		result.reserve(mMeshes.size());
		for (std::size_t i = 0; i < mMeshes.size(); ++i)
			result.emplace_back(std::make_pair(static_cast<MeshID>(i), mMeshes[i]));

		return result;
	}

	[[nodiscard]] std::vector<std::pair<MaterialID, MaterialData>> materials() const noexcept override {
		std::vector<std::pair<MaterialID, MaterialData>> result;
		result.reserve(mMaterials.size());
		for (std::size_t i = 0; i < mMaterials.size(); ++i)
			result.emplace_back(std::make_pair(static_cast<MaterialID>(i), mMaterials[i]));

		return result;
	}

	[[nodiscard]] std::vector<std::pair<TextureID, TextureData>> textures() const noexcept override {
		std::vector<std::pair<TextureID, TextureData>> result;
		result.reserve(mTextures.size());
		for (std::size_t i = 0; i < mTextures.size(); ++i)
			result.emplace_back(std::make_pair(static_cast<TextureID>(i), mTextures[i]));

		return result;
	}

private:

	std::size_t mVertexCount = 0;
	std::size_t mIndexCount	 = 0;
	Size2		mLargestTexture {.width=0, .height=0};

	std::vector<MeshData>	  mMeshes;
	std::vector<MaterialData> mMaterials;
	std::vector<TextureData>  mTextures;
};

} // namespace Lithe
