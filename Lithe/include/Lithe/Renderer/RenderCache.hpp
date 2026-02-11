#pragma once

#include "IDs.hpp"

#include <set>
#include <unordered_map>
#include <vector>

namespace Lithe {

struct MeshAllocation {
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexCount;
	uint32_t firstIndex;
};

struct RenderKey {
	static constexpr int MESH_BITS	  = sizeof(MeshID) * 8;
	static constexpr int TEXTURE_BITS = sizeof(TextureID) * 8;

	uint64_t key;

	RenderKey(MaterialID materialID, TextureID textureID, MeshID meshID) {
		key = (uint64_t(materialID) << (TEXTURE_BITS + MESH_BITS)) | (uint64_t(textureID) << MESH_BITS) |
			  uint64_t(meshID);
	}

	operator uint64_t() const noexcept { return key; }

	auto operator==(const RenderKey& other) const noexcept { return key == other.key; }

	auto operator<=>(const RenderKey& other) const noexcept { return key <=> other.key; }
};

// TODO: need to improve a bit on safety
class RenderCache {

public:

	[[nodiscard]] bool hasMesh(MeshID id) const noexcept { return mMeshAllocations.find(id) != mMeshAllocations.end(); }

	[[nodiscard]] bool hasMaterial(MaterialID id) const noexcept {
		return mMaterialAllocations.find(id) != mMaterialAllocations.end();
	}

	[[nodiscard]] bool hasTexture(TextureID id) const noexcept {
		return mTextureAllocations.find(id) != mTextureAllocations.end();
	}

	[[nodiscard]] bool addMesh(MeshID id, const MeshAllocation& alloc) noexcept {
		mMeshAllocations[id] = alloc;

		return true;
	}

	[[nodiscard]] bool addMaterial(MaterialID id) noexcept {
		mMaterialAllocations.insert(id);

		return true;
	}

	[[nodiscard]] bool addTexture(TextureID id) noexcept {
		mTextureAllocations.insert(id);

		return true;
	}

	[[nodiscard]] bool addKey(const RenderKey& key) noexcept {
		auto it = std::upper_bound(mKeys.begin(), mKeys.end(), key, [](const RenderKey& a, const RenderKey& b) {
			return a < b;
		});

		mKeys.insert(it, key);

		return true;
	}

	[[nodiscard]] std::vector<RenderKey> keys() const noexcept { return mKeys; }

	void clearAll() {
		mMeshAllocations.clear();
		mMaterialAllocations.clear();
		mTextureAllocations.clear();
		mKeys.clear();
	}

	[[nodiscard]] auto& operator[](std::size_t index) const noexcept { return mKeys[index]; }

	[[nodiscard]] auto operator[](MeshID id) const noexcept { return mMeshAllocations.at(id); }

private:

	std::unordered_map<MeshID, MeshAllocation> mMeshAllocations;
	std::set<MaterialID>					   mMaterialAllocations;
	std::set<TextureID>						   mTextureAllocations;
	std::vector<RenderKey>					   mKeys;
};

} // namespace Lithe

