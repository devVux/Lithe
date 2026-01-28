#pragma once

#include "ForwardDecls.hpp"
#include "ISurface.hpp"

#include <optional>
#include <set>
#include <string>

namespace Lithe {

using Extension = std::string;

class RenderSystem {

public:

	~RenderSystem() noexcept;
	bool init(ISurface&, std::set<Extension> = {});

	void render();

private:

	RAIIed<VkInstance> mInstance;
	RAIIed<VkDevice>   mDevice;

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
