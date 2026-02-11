#pragma once

#include "EventDispatcher.hpp"
#include "ForwardDecls.hpp"
#include "IDs.hpp"

#include <cstdint>

namespace Lithe {

struct DebugInfo {
	double	 delta;
	uint32_t drawCalls;
	uint32_t instanceCount;
	uint32_t mergedBatches;

	uint32_t triangleCount;
	uint32_t vertexCount;

	std::unordered_map<MaterialID, uint32_t> materialDrawCalls;
	std::unordered_map<MeshID, uint32_t>	 meshDrawCalls;

	void reset() noexcept {
		delta = 0;

		drawCalls	  = 0;
		instanceCount = 0;
		mergedBatches = 0;

		triangleCount = 0;
		vertexCount	  = 0;

		materialDrawCalls.clear();
		meshDrawCalls.clear();
	}
};

class DebugLayer {
public:

	~DebugLayer() noexcept;

	void init(
		EventDispatcher&,
		VkInstance		 instance,
		VkPhysicalDevice physicalDevice,
		VkDevice		 device,
		VkQueue			 graphicsQueue,
		uint32_t		 queueFamily,
		LtFormat		 colorFormat,
		uint32_t		 minImageCount,
		uint32_t		 imageCount,
		Size			 windowSize
	);

	void update(VkCommandBuffer cmd, const DebugInfo& info) noexcept;

private:

	RAIIed<VkDescriptorPool> mDescriptorPool;
};

} // namespace Lithe
