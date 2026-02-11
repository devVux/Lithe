#pragma once

#include "Allocator.hpp"
#include "DebugLayer.hpp"
#include "ForwardDecls.hpp"
#include "IResourceCache.hpp"
#include "ISurface.hpp"
#include "RenderPacket.hpp"

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
	bool init(EventDispatcher&, ISurface&, std::set<Extension> = {}) noexcept;

	bool uploadStaticData(StaticRenderPacket&, IResourceCache&) noexcept;
	void render(DynamicRenderPacket&, IResourceCache&) noexcept;
	void renderOverlay() noexcept;

private:

	RAIIed<VkInstance> mInstance;
	RAIIed<VkDevice>   mDevice;
	VkPhysicalDevice   mPhysicalDevice;

	QueueFamilyIndices mIndices;

	RAIIed<VkSurfaceKHR>   mSurface;
	RAIIed<VkSwapchainKHR> mSwapchain;

	RAIIed<VkPipeline>						   mPipeline;
	RAIIed<VkPipelineLayout>				   mPipelineLayout;
	RAIIed<std::vector<VkDescriptorSetLayout>> mDescriptorSetLayouts;

	RAIIed<VkDescriptorPool>				  mDescriptorPool;
	std::vector<VkDescriptorSet>			  mPersistentDescriptorSets;
	std::vector<std::vector<VkDescriptorSet>> mDynamicDescriptorSets;

	RAIIed<VkCommandPool> mCommandPool;

	struct FrameResources {
		VkCommandBuffer scene;
		VkCommandBuffer overlay;
	};

	std::vector<FrameResources> mCommandBuffers;

	std::vector<VkImage>			 mSwapchainImages;
	std::vector<RAIIed<VkImageView>> mSwapchainImageViews;
	std::vector<VkImage>			 mImages;
	std::vector<RAIIed<VkImageView>> mImageViews;

	Buffer				mVertexBuffer;
	Buffer				mIndexBuffer;
	Buffer				mStorageBuffer;
	Buffer				mMaterialBuffer;
	std::vector<Buffer> mUniformBuffers;

	RAIIed<VkSampler> mGlobalSampler;

	VkQueue mGraphicsQueue;
	VkQueue mPresentQueue;
	VkQueue mTransferQueue;

	std::vector<RAIIed<VkSemaphore>> mImageAvailableSemaphore;
	std::vector<RAIIed<VkSemaphore>> mRenderFinishedSemaphore;
	std::vector<RAIIed<VkFence>>	 mInFlightFence;
	std::vector<RAIIed<VkSemaphore>> mSceneRenderedSemaphore;

	Allocator mAllocator;

	std::vector<Image>				 mDepthImages;
	std::vector<RAIIed<VkImageView>> mDepthImageViews;

	DebugLayer debugLayer;
};

} // namespace Lithe
