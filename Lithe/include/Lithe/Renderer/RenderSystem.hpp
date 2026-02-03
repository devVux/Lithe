#pragma once

#include "ForwardDecls.hpp"
#include "ISurface.hpp"
#include "RenderPacket.hpp"
#include "IResourceCache.hpp"
#include "Allocator.hpp"

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

	bool uploadStaticData(StaticRenderPacket&, IResourceCache&) noexcept;
	void render(DynamicRenderPacket&, IResourceCache&);

private:

	RAIIed<VkInstance> mInstance;
	RAIIed<VkDevice>   mDevice;
	VkPhysicalDevice   mPhysicalDevice;

	QueueFamilyIndices mIndices;

	RAIIed<VkSurfaceKHR>   mSurface;
	RAIIed<VkSwapchainKHR> mSwapchain;

	RAIIed<VkPipeline>		 mPipeline;
	RAIIed<VkPipelineLayout> mPipelineLayout;
	RAIIed<VkDescriptorSetLayout> mDescriptorSetLayout;

	RAIIed<VkCommandPool>		 mCommandPool;
	std::vector<VkCommandBuffer> mCommandBuffers;

	std::vector<VkImage>			 mImages;
	std::vector<RAIIed<VkImageView>> mImageViews;

	Buffer mVertexBuffer;
	Buffer mIndexBuffer;
	std::vector<Buffer> mUniformBuffers;

	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;

	std::vector<RAIIed<VkSemaphore>> mImageAvailableSemaphore;
	std::vector<RAIIed<VkSemaphore>> mRenderFinishedSemaphore;
	std::vector<RAIIed<VkFence>>	 mInFlightFence;

	RAIIed<VkDescriptorPool> mDescriptorPool;
	std::vector<VkDescriptorSet> mDescriptorSets;

	Allocator mAllocator;

	std::vector<Image> mDepthImages;
	std::vector<RAIIed<VkImageView>> mDepthImageViews;

};

} // namespace Lithe
