#pragma once

#include "ForwardDecls.hpp"
#include "ISurface.hpp"

#include <optional>
#include <set>
#include <string>

namespace Lithe {

using Extension = std::string;


struct QueueFamilyIndices {
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> computeFamily;
	std::optional<uint32_t> transferFamily;
	std::optional<uint32_t> presentFamily;

	bool covers(uint32_t requiredQueues, bool needsPreset) const noexcept;
};


class RenderSystem {

public:

	~RenderSystem() noexcept;
	bool init(ISurface&, std::set<Extension> = {});

	void render();

private:

	RAIIed<VkInstance> mInstance;
	RAIIed<VkDevice>   mDevice;
	VkPhysicalDevice   mPhysicalDevice;

	QueueFamilyIndices mIndices;

	RAIIed<VkSurfaceKHR>   mSurface;
	RAIIed<VkSwapchainKHR> mSwapchain;

	RAIIed<VkPipeline>		 mPipeline;
	RAIIed<VkPipelineLayout> mPipelineLayout;

	RAIIed<VkCommandPool>		 mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;

	std::vector<VkImage>			 mImages;
	std::vector<RAIIed<VkImageView>> mImageViews;

	RAIIed<VkDeviceMemory> mMemory;
	RAIIed<VkBuffer>	   mVertexBuffer;
	RAIIed<VkBuffer>	   mIndexBuffer;

	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	std::vector<RAIIed<VkSemaphore>> mImageAvailableSemaphore;
	std::vector<RAIIed<VkSemaphore>> mRenderFinishedSemaphore;
	std::vector<RAIIed<VkFence>>	 mInFlightFence;
};

} // namespace Lithe
