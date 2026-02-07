#include "RenderSystem.hpp"

#include "ForwardDecls.hpp"
#include "Log.hpp"
#include "SurfaceFactory.hpp"
#include "RenderCache.hpp"
//#include "DebugLayer.hpp"

#include "Inits.cpp"


#include <algorithm>
#include <cassert>
#include <expected>
#include <fstream>
#include <optional>
#include <set>
#include <vector>
#include <ranges>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vma/vk_mem_alloc.h>
#include <glm/ext/matrix_transform.hpp>


constexpr size_t operator""_KB(unsigned long long v) {
	return v * 1'024;
}

constexpr size_t operator""_MB(unsigned long long v) {
	return v * 1'024 * 1'024;
}

constexpr size_t operator""_GB(unsigned long long v) {
	return v * 1'024 * 1'024 * 1'024;
}

//static Lithe::DebugLayer debugLayer;


namespace Render {

struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

using Index = uint32_t;

struct CameraUBO {
	glm::mat4 viewProjection;
};

struct alignas(16) MaterialData {
	glm::vec4 color;
	uint32_t albedoTextureIndex;
	uint32_t pad[3];
};


struct alignas(16) InstanceData {
	glm::mat4 model;
	uint32_t materialIndex;
	uint32_t textureIndex;
	uint32_t pad[2];
};

}

static int currentFrame	   = 0;
static int nFramesInFlight = 2;

namespace Lithe {

RenderSystem::~RenderSystem() noexcept {
	vkDeviceWaitIdle(mDevice);


}

bool RenderSystem::init(ISurface& surface, std::set<Extension> extensions) noexcept {

	extensions.insert("VK_KHR_surface");

#ifdef LT_DEBUG
	std::set<std::string> layers {"VK_LAYER_KHRONOS_validation"};

	extensions.insert("VK_EXT_debug_utils");

#else
	std::set<std::string> layers {};
#endif
	bool hasValidationLayer = !layers.empty() && Validation::satisfies(layers, {"VK_LAYER_KHRONOS_validation"});

	LT_LOG_DEBUG("Vulkan Extensions:");
	for (const auto& ext : extensions)
		LT_LOG_DEBUG("  - {}", ext);

	LT_LOG_DEBUG("Vulkan Layers:");
	for (const auto& layer : layers)
		LT_LOG_DEBUG("  - {}", layer);

	
	std::expected<InitContext, E> result;

{

	result =
		Validation::createInstance(extensions, layers, hasValidationLayer)
			.and_then([&](auto result) -> std::expected<InitContext, E> {
				auto [instance, ctx] = result;
				LT_LOG_INFO("Vulkan instance created");

				mInstance = RAIIed<VkInstance>(instance, [hasValidationLayer, ctx](VkInstance instance) noexcept {
					if (hasValidationLayer)
						Validation::destroyDebugUtilsMessenger(instance, ctx.debugMessenger, nullptr);

					if (instance)
						vkDestroyInstance(instance, nullptr);
				});

				return ctx;
			})
			.and_then([&](InitContext ctx) -> std::expected<InitContext, E> {
				LT_LOG_TRACE("Creating surface");

				auto surf = SurfaceFactory::createSurface(mInstance, surface);
				if (!surf)
					return std::unexpected(surf.error());

				mSurface = RAIIed<VkSurfaceKHR>(
					*surf, [instance = static_cast<VkInstance>(mInstance)](VkSurfaceKHR surface) noexcept {
						if (surface)
							vkDestroySurfaceKHR(instance, surface, nullptr);
					}
				);

				LT_LOG_INFO("Surface created");
				return ctx;
			})
			.and_then([&](InitContext ctx) -> std::expected<InitContext, E> {
				LT_LOG_TRACE("Selecting physical device");

				return Device::enumeratePhysicalDevices(mInstance)
					.and_then([&](auto devices) {
						return Device::selectPhysicalDevice(devices, mSurface, VK_QUEUE_GRAPHICS_BIT);
					})
					.transform([&](VkPhysicalDevice device) {
						mPhysicalDevice = device;
						mIndices		= Device::findQueueFamilies(device, mSurface);
						LT_LOG_INFO("Physical device selected");
						return ctx;
					});
			})
			.and_then([&](InitContext ctx) -> std::expected<InitContext, E> {
				LT_LOG_TRACE("Creating logical device");

				// TODO: provide required queues
				auto device = Device::create(mPhysicalDevice, mIndices);
				if (!device)
					return std::unexpected(device.error());

				mDevice = RAIIed<VkDevice>(*device, [](VkDevice device) noexcept {
					if (device)
						vkDestroyDevice(device, nullptr);
				});

				vkGetDeviceQueue(mDevice, *mIndices.graphicsFamily, 0, &mGraphicsQueue);
				vkGetDeviceQueue(mDevice, *mIndices.presentFamily, 0, &mPresentQueue);
				vkGetDeviceQueue(mDevice, *mIndices.transferFamily, 0, &mTransferQueue);

				LT_LOG_INFO("Logical device + queues ready");
				return ctx;
			})
			.and_then([&](InitContext ctx) -> std::expected<InitContext, E> {
				LT_LOG_TRACE("Creating swapchain");

				auto result = Swapchain::create(mPhysicalDevice, mDevice, mSurface, ctx.extent, mIndices);
				if (!result)
					return std::unexpected(result.error());

				mSwapchain = RAIIed<VkSwapchainKHR>(
					result->swapchain, [device = static_cast<VkDevice>(mDevice)](VkSwapchainKHR sc) noexcept {
						if (sc)
							vkDestroySwapchainKHR(device, sc, nullptr);
					}
				);

				ctx.surfaceFormat = result->surfaceFormat;
				ctx.extent		  = result->extent;

				LT_LOG_INFO("Swapchain created");
				return ctx;
			})
			.and_then([&](InitContext ctx) -> std::expected<InitContext, E> {
				uint32_t count;
				vkGetSwapchainImagesKHR(mDevice, mSwapchain, &count, nullptr);
				mSwapchainImages.resize(count);
				vkGetSwapchainImagesKHR(mDevice, mSwapchain, &count, mSwapchainImages.data());

				LT_LOG_TRACE("Creating {} image views", count);

				mSwapchainImageViews.reserve(count);

				for (auto img : mSwapchainImages) {
					VkImageView			  view;
					VkImageViewCreateInfo info {
						.sType			  = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
						.image			  = img,
						.viewType		  = VK_IMAGE_VIEW_TYPE_2D,
						.format			  = VK_FORMAT_B8G8R8A8_UNORM,
						.subresourceRange = {
											 .aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
											 .baseMipLevel	= 0,
											 .levelCount		= 1,
											 .baseArrayLayer = 0,
											 .layerCount		= 1
						}
					};

					if (vkCreateImageView(mDevice, &info, nullptr, &view) != VK_SUCCESS)
						return std::unexpected(E::Unknown);

					mSwapchainImageViews.emplace_back(
						view, [device = static_cast<VkDevice>(mDevice)](VkImageView view) noexcept {
							if (view)
								vkDestroyImageView(device, view, nullptr);
						}
					);
				}

				LT_LOG_INFO("Image views created");
				return ctx;
			})
			.and_then([&](InitContext ctx) -> std::expected<InitContext, E> {
				LT_LOG_TRACE("Creating pipeline");
				auto [descriptor, layout, pipe] = Pipeline::createPipeline(mDevice, ctx.extent, ctx.surfaceFormat.format);

				if (!layout || !pipe)
					return std::unexpected(E::Unknown);

				mDescriptorSetLayouts = RAIIed<std::vector<VkDescriptorSetLayout>>(
					descriptor, [device = static_cast<VkDevice>(mDevice)](std::vector<VkDescriptorSetLayout> setLayouts) noexcept {
						for (auto& layout : setLayouts)
							vkDestroyDescriptorSetLayout(device, layout, nullptr);
					}
				);

				mPipelineLayout = RAIIed<VkPipelineLayout>(
					*layout, [device = static_cast<VkDevice>(mDevice)](VkPipelineLayout layout) noexcept {
						if (layout)
							vkDestroyPipelineLayout(device, layout, nullptr);
					}
				);

				mPipeline =
					RAIIed<VkPipeline>(*pipe, [device = static_cast<VkDevice>(mDevice)](VkPipeline pipeline) noexcept {
						if (pipeline)
							vkDestroyPipeline(device, pipeline, nullptr);
					});

				LT_LOG_INFO("Graphics pipeline ready");
				return ctx;
			})
			.and_then([&](InitContext ctx) -> std::expected<InitContext, E> {
				LT_LOG_TRACE("Creating command pool");

				VkCommandPoolCreateInfo poolInfo {};
				poolInfo.sType			  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				poolInfo.flags			  = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				poolInfo.queueFamilyIndex = mIndices.graphicsFamily.value();

				VkCommandPool commandPool;
				if (vkCreateCommandPool(mDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
					return std::unexpected {E::Unknown};

				mCommandPool = RAIIed<VkCommandPool>(
					commandPool, [device = static_cast<VkDevice>(mDevice)](VkCommandPool pool) noexcept {
						if (pool)
							vkDestroyCommandPool(device, pool, nullptr);
					}
				);

				LT_LOG_INFO("Command pool created");
				return ctx;
			})
			.and_then([&](InitContext ctx) -> std::expected<InitContext, E> {
				LT_LOG_TRACE("Creating command buffers");

				mCommandBuffers.resize(nFramesInFlight);

				VkCommandBufferAllocateInfo allocInfo {};
				allocInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
				allocInfo.commandPool		 = mCommandPool;
				allocInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
				allocInfo.commandBufferCount = static_cast<uint32_t>(mCommandBuffers.size());

				if (vkAllocateCommandBuffers(mDevice, &allocInfo, mCommandBuffers.data()) != VK_SUCCESS)
					return std::unexpected {E::Unknown};

				LT_LOG_INFO("Command buffers ready");
				return ctx;
			})
			.or_else([](E e) -> std::expected<InitContext, E> {
				LT_LOG_CRITICAL("Renderer init failed");
				return std::unexpected(e);
			});

	if (!result)
		return false;

}


	mAllocator = Allocator(mPhysicalDevice, mDevice, mInstance);

	mVertexBuffer =
		mAllocator.createBuffer(1_MB, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Mapped::Yes);

	mIndexBuffer =
		mAllocator.createBuffer(1_MB, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Mapped::Yes);

	mMaterialBuffer =
		mAllocator.createBuffer(10 * sizeof(Render::MaterialData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Mapped::Yes);

	mStorageBuffer =
		mAllocator.createBuffer(100 * sizeof(Render::InstanceData), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Mapped::Yes);

	mUniformBuffers.resize(nFramesInFlight);
	for (size_t i = 0; i < nFramesInFlight; i++)
		mUniformBuffers[i] = mAllocator.createBuffer(sizeof(Render::CameraUBO), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Mapped::Yes);



	VkSemaphoreCreateInfo semInfo = {
		.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
	};

	for (int i = 0; i < mSwapchainImages.size(); i++) {
		VkSemaphore renderFinishedSemaphore;
		vkCreateSemaphore(mDevice, &semInfo, NULL, &renderFinishedSemaphore);

		mRenderFinishedSemaphore.push_back(
			RAIIed<VkSemaphore>(
				renderFinishedSemaphore, [device = static_cast<VkDevice>(mDevice)](auto semaphore) noexcept {
					if (semaphore)
						vkDestroySemaphore(device, semaphore, nullptr);
				}
			)
		);
 

		VkSemaphore imageAvailableSemaphore;
		vkCreateSemaphore(mDevice, &semInfo, NULL, &imageAvailableSemaphore);

		mImageAvailableSemaphore.push_back(
			RAIIed<VkSemaphore>(
				imageAvailableSemaphore, [device = static_cast<VkDevice>(mDevice)](auto semaphore) noexcept {
					if (semaphore)
						vkDestroySemaphore(device, semaphore, nullptr);
				}
			)
		);

		VkFenceCreateInfo fenceInfo = {
		.sType				= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		.flags				= VK_FENCE_CREATE_SIGNALED_BIT,
		};
		VkFence inFlightFence;
		vkCreateFence(mDevice, &fenceInfo, NULL, &inFlightFence);
		mInFlightFence.push_back(
			RAIIed<VkFence>(inFlightFence, [device = static_cast<VkDevice>(mDevice)](auto fence) noexcept {
				if (fence)
					vkDestroyFence(device, fence, nullptr);
			})
		);

	}



	std::vector<VkDescriptorPoolSize> poolSizes {
		// set 0
		{
			.type			 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.descriptorCount = static_cast<uint32_t>(nFramesInFlight)
		},


		// set 1
		{.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = 1 },
		{.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = 1 * 10 },
		{.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = 1 },


		// set 2
		{
			.type			 = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
			.descriptorCount = 1
		},
	};

	VkDescriptorPoolCreateInfo poolInfo {};
	poolInfo.sType		   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes	   = poolSizes.data();
	poolInfo.maxSets	   = static_cast<uint32_t>(nFramesInFlight * 3);

	VkDescriptorPool descriptorPool;
	assert(vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS);

	// 3 sets total
	mDynamicDescriptorSets.resize(nFramesInFlight, std::vector<VkDescriptorSet>(1));
	mUniformBuffers.resize(nFramesInFlight);

	{	// set 0

		for (auto i = 0; i < nFramesInFlight; i++) {

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &mDescriptorSetLayouts.get()[0];

			vkAllocateDescriptorSets(mDevice, &allocInfo, &mDynamicDescriptorSets[i][0]);


			// Camera UBO
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers[i].handle;
			bufferInfo.offset = 0;
			bufferInfo.range = VK_WHOLE_SIZE;

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mDynamicDescriptorSets[i][0];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);

		}

	}


	mPersistentDescriptorSets.resize(3, VK_NULL_HANDLE);
	// We leave mPersistentDescriptorSets[set = 0] null for clarity.
	// Otherwise, referencing it as 0 could be mistaken for the per-frame (dynamic) descriptor set.

	{	// Set 1
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mDescriptorSetLayouts.get()[1];

		vkAllocateDescriptorSets(mDevice, &allocInfo, &mPersistentDescriptorSets[1]);


		// Material SSBO
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mMaterialBuffer.handle;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mPersistentDescriptorSets[1];
		descriptorWrite.dstBinding = 2;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
	}
	
	{	// Set 2
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &mDescriptorSetLayouts.get()[2];

		vkAllocateDescriptorSets(mDevice, &allocInfo, &mPersistentDescriptorSets[2]);


		// Instance SSBO
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = mStorageBuffer.handle;
		bufferInfo.offset = 0;
		bufferInfo.range = VK_WHOLE_SIZE;

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = mPersistentDescriptorSets[2];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);
	}



	mDescriptorPool = RAIIed<VkDescriptorPool>(
		descriptorPool, [device = static_cast<VkDevice>(mDevice)](VkDescriptorPool pool) noexcept {
			if (pool)
				vkDestroyDescriptorPool(device, pool, nullptr);
		}
	);


	mDepthImages.resize(nFramesInFlight);
	mDepthImageViews.resize(nFramesInFlight);

	for (auto i = 0; i < nFramesInFlight; i++) {

		auto depthImage = mAllocator.createDepthImage(result->extent.width, result->extent.height);

		VkImageViewCreateInfo depthViewInfo{
			.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = depthImage.handle,
			.viewType = VK_IMAGE_VIEW_TYPE_2D,
			.format = VK_FORMAT_D32_SFLOAT,
			.subresourceRange = {
				.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
				.baseMipLevel = 0,
				.levelCount = 1,
				.baseArrayLayer = 0,
				.layerCount = 1
			}
		};

		VkImageView depthImageView;
		assert(vkCreateImageView(mDevice, &depthViewInfo, nullptr, &depthImageView) == VK_SUCCESS);


		mDepthImages[i] = depthImage;

		mDepthImageViews[i] = RAIIed<VkImageView>(depthImageView, [device = static_cast<VkDevice>(mDevice)](auto view) noexcept {
			if (view)
				vkDestroyImageView(device, view, nullptr);
		});

	}


	return true;
}


RenderCache renderCache;

bool RenderSystem::uploadStaticData(StaticRenderPacket& statics, IResourceCache& cache) noexcept {
	
	std::vector<Render::Vertex> vertices;
	std::vector<Render::Index> indices;
	std::vector<Render::MaterialData> materials;
	std::vector<Render::InstanceData> instances;

	
	vertices.reserve(cache.vertexCount());
	indices.reserve(cache.indexCount());
	materials.reserve(cache.materialCount());
	instances.reserve(statics.instances.size());


	for (const auto& [id, mesh] : cache.meshes()) {
		renderCache.addMesh(id, MeshAllocation {
			.vertexOffset = static_cast<uint32_t>(vertices.size()),
			.indexCount = static_cast<uint32_t>(mesh.indices.size()),
			.firstIndex = static_cast<uint32_t>(indices.size()),
		});


		for (size_t i = 0; i < mesh.positions.size(); ++i) {
			vertices.emplace_back(Render::Vertex{
				.position = mesh.positions[i],
				//.normal = mesh.normals[i],
				.uv = mesh.uvs[i]
			});
		}

		indices.insert(std::end(indices), std::begin(mesh.indices), std::end(mesh.indices));
	}




	for (const auto& [id, material] : cache.materials()) {

		materials.emplace_back(Render::MaterialData {
			.color = material.color,
			.albedoTextureIndex = static_cast<uint32_t>(material.albedoTextureID)
		});


		renderCache.addMaterial(id);
	}

	uint32_t maxWidth = cache.largestTexture().width;
	uint32_t maxHeight = cache.largestTexture().height;



	// Temporary allocator for staging textures
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = mPhysicalDevice;
	allocatorInfo.device = mDevice;
	allocatorInfo.instance = mInstance;
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

	VmaAllocator tempAllocator;
	vmaCreateAllocator(&allocatorInfo, &tempAllocator);

	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = maxWidth * maxHeight * sizeof(float);
	bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;


	Buffer stagingBuffer;
	VmaAllocationInfo allocationInfo;
	vmaCreateBuffer(tempAllocator, &bufferInfo, &allocInfo, &stagingBuffer.handle, &stagingBuffer.allocation, &allocationInfo);

	stagingBuffer.mapped = allocationInfo.pMappedData;



	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = mIndices.transferFamily.value();

	VkCommandPool transferCmdPool;
	assert(vkCreateCommandPool(mDevice, &poolInfo, nullptr, &transferCmdPool) == VK_SUCCESS);


	VkCommandBufferAllocateInfo transferInfo{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.pNext = nullptr,
		.commandPool = transferCmdPool,
		.commandBufferCount = 1,
	};
	
	VkCommandBuffer transferCmd;
	assert(vkAllocateCommandBuffers(mDevice, &transferInfo, &transferCmd) == VK_SUCCESS);


	for (const auto& [id, texture] : cache.textures()) {

		auto image = mAllocator.createImage(
			texture.width,
			texture.height,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT
		);

		assert(texture.pixels.size() == texture.width * texture.height * 4);
		std::memcpy(stagingBuffer.mapped, texture.pixels.data(), texture.pixels.size());

		vkResetCommandBuffer(transferCmd, 0);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(transferCmd, &beginInfo);

		VkImageMemoryBarrier barrierToDst{};
		barrierToDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierToDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrierToDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrierToDst.srcAccessMask = 0;
		barrierToDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrierToDst.image = image.handle;
		barrierToDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrierToDst.subresourceRange.baseMipLevel = 0;
		barrierToDst.subresourceRange.levelCount = 1;
		barrierToDst.subresourceRange.baseArrayLayer = 0;
		barrierToDst.subresourceRange.layerCount = 1;
		barrierToDst.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrierToDst.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vkCmdPipelineBarrier(
			transferCmd,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrierToDst
		);

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = { texture.width, texture.height, 1 };

		vkCmdCopyBufferToImage(
			transferCmd,
			stagingBuffer.handle,
			image.handle,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&region
		);

		VkImageMemoryBarrier barrierToShader{};
		barrierToShader.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrierToShader.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrierToShader.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrierToShader.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrierToShader.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		barrierToShader.image = image.handle;
		barrierToShader.subresourceRange = barrierToDst.subresourceRange;
		barrierToShader.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrierToShader.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

		vkCmdPipelineBarrier(
			transferCmd,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrierToShader
		);

		vkEndCommandBuffer(transferCmd);

		VkSubmitInfo submit{};
		submit.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit.commandBufferCount = 1;
		submit.pCommandBuffers = &transferCmd;
		vkQueueSubmit(mTransferQueue, 1, &submit, VK_NULL_HANDLE);
		vkQueueWaitIdle(mTransferQueue);

		renderCache.addTexture(id);
		mImages.emplace_back(image.handle);
	}


	vkDestroyCommandPool(mDevice, transferCmdPool, nullptr);
	vmaDestroyBuffer(tempAllocator, stagingBuffer.handle, stagingBuffer.allocation);
	vmaDestroyAllocator(tempAllocator);

	for (auto& image : mImages) {

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		VkImageView imageView;
		assert(vkCreateImageView(mDevice, &viewInfo, nullptr, &imageView) == VK_SUCCESS);

		mImageViews.emplace_back(
			RAIIed<VkImageView>(imageView, [device = static_cast<VkDevice>(mDevice)](VkImageView view) noexcept {
				if (view)
					vkDestroyImageView(device, view, nullptr);
			})
		);

	}



	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_FALSE;
	samplerInfo.maxAnisotropy = 1.0f;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	VkSampler sampler;
	vkCreateSampler(mDevice, &samplerInfo, nullptr, &sampler);

	VkDescriptorImageInfo samplerDescriptor{};
	samplerDescriptor.sampler = sampler;

	VkWriteDescriptorSet samplerWrite{};
	samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	samplerWrite.dstSet = mPersistentDescriptorSets[1];
	samplerWrite.dstBinding = 0;
	samplerWrite.dstArrayElement = 0;
	samplerWrite.descriptorCount = 1;
	samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	samplerWrite.pImageInfo = &samplerDescriptor;

	vkUpdateDescriptorSets(mDevice, 1, &samplerWrite, 0, nullptr);

	for (int i = 0; i < cache.textureCount(); i++) {
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = mImageViews[i];
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstSet = mPersistentDescriptorSets[1];
		write.dstBinding = 1;
		write.dstArrayElement = i;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		write.pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(mDevice, 1, &write, 0, nullptr);

	}




	mGlobalSampler = RAIIed<VkSampler>(sampler, [device = static_cast<VkDevice>(mDevice)](VkSampler sampler) noexcept {
		if (sampler)
			vkDestroySampler(device, sampler, nullptr);
		});




	for (const auto& entity : statics.instances) {

		instances.emplace_back(Render::InstanceData {
			.model = entity.model,
			.materialIndex = entity.materialID,
			.textureIndex = entity.textureID
		});


		renderCache.addKey(RenderKey(entity.materialID, entity.textureID, entity.meshID));

	}


	std::memcpy(mVertexBuffer.mapped, vertices.data(), vertices.size() * sizeof(Render::Vertex));
	std::memcpy(mIndexBuffer.mapped, indices.data(), indices.size() * sizeof(Render::Index));
	std::memcpy(mMaterialBuffer.mapped, materials.data(), materials.size() * sizeof(Render::MaterialData));
	std::memcpy(mStorageBuffer.mapped, instances.data(), instances.size() * sizeof(Render::InstanceData));

	return true;
}


void RenderSystem::render(DynamicRenderPacket& dynamics, IResourceCache& cache) noexcept {
	auto cmd = mCommandBuffers.at(currentFrame);


	vkWaitForFences(mDevice, 1, &static_cast<const VkFence&>(mInFlightFence[currentFrame]), VK_TRUE, UINT64_MAX);
	vkResetFences(mDevice, 1, &static_cast<const VkFence&>(mInFlightFence[currentFrame]));

	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR(
		mDevice, mSwapchain, UINT64_MAX, mImageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex
	);

	VkCommandBufferBeginInfo cmdBeginInfo {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, .flags = 0, .pInheritanceInfo = nullptr
	};
	vkBeginCommandBuffer(cmd, &cmdBeginInfo);

	VkImageMemoryBarrier depthBarrier{};
	depthBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	depthBarrier.srcAccessMask = 0;
	depthBarrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
	depthBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthBarrier.newLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	depthBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	depthBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	depthBarrier.image = mDepthImages[currentFrame].handle;
	depthBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthBarrier.subresourceRange.baseMipLevel = 0;
	depthBarrier.subresourceRange.levelCount = 1;
	depthBarrier.subresourceRange.baseArrayLayer = 0;
	depthBarrier.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		cmd,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &depthBarrier
	);

	VkImageMemoryBarrier barrier {};
	barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask					= 0;
	barrier.dstAccessMask					= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.oldLayout						= VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.image							= mSwapchainImages[imageIndex];
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount		= 1;

	vkCmdPipelineBarrier(
		cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0,
		nullptr, 1, &barrier
	);

	VkRenderingAttachmentInfo colorAttachment = {};
	colorAttachment.sType					  = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	colorAttachment.imageView				  = mSwapchainImageViews.at(imageIndex);
	colorAttachment.imageLayout				  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	colorAttachment.loadOp					  = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp					  = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.clearValue.color		  = {
		 {0.0f, 0.0f, 0.0f, 1.0f}
	 };

	 VkRenderingAttachmentInfo depthAttachment = {};
	 depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	 depthAttachment.imageView = mDepthImageViews[currentFrame];
	 depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	 depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	 depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	 depthAttachment.clearValue.depthStencil = {1.0f, 0};

	VkExtent2D size {800, 600};	// TODO: replace with swapchain size

	VkRenderingInfo renderingInfo	   = {};
	renderingInfo.sType				   = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea.offset	   = {0, 0};
	renderingInfo.renderArea.extent	   = size;
	renderingInfo.layerCount		   = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments	   = &colorAttachment;
	renderingInfo.pDepthAttachment = &depthAttachment;

	vkCmdBeginRendering(cmd, &renderingInfo);




	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);

	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = (float)size.height;
	viewport.width = (float)size.width;
	viewport.height = -1 * (float)size.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D   scissor	= {
		   {			0,		   0},
		   {size.width, size.height}
	   };
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);



	std::memcpy(mUniformBuffers[currentFrame].mapped, &dynamics.camera, sizeof(Render::CameraUBO));

	// Camera UBO
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = mUniformBuffers[currentFrame].handle;
	bufferInfo.offset = 0;
	bufferInfo.range = VK_WHOLE_SIZE;

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = mDynamicDescriptorSets[currentFrame][0];
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);






	vkCmdBindDescriptorSets(
		cmd, 
		VK_PIPELINE_BIND_POINT_GRAPHICS, 
		mPipelineLayout, 
		0, 
		mDynamicDescriptorSets[currentFrame].size(),
		mDynamicDescriptorSets[currentFrame].data(),
		0, 
		nullptr
	);

	vkCmdBindDescriptorSets(
		cmd, 
		VK_PIPELINE_BIND_POINT_GRAPHICS, 
		mPipelineLayout, 
		1, 
		mPersistentDescriptorSets.size() - 1,	// except set 0
		mPersistentDescriptorSets.data() + 1,
		0, 
		nullptr
	);


	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(cmd, 0, 1, &mVertexBuffer.handle, offsets);
	vkCmdBindIndexBuffer(cmd, mIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);


	std::size_t instanceCount;

	std::size_t firstIstance = 0;
	std::size_t lastInstance;
	while (firstIstance < renderCache.keys().size()) {
		RenderKey key = renderCache[firstIstance];
	
		MaterialID materialID = (key >> 32) & 0xFFFF;
		MeshID meshID = key & 0xFFFFFFFF;

		const auto& meshAlloc = renderCache[meshID];

		lastInstance = firstIstance;
		while (lastInstance < renderCache.keys().size() && renderCache[lastInstance] == key)
			lastInstance++;

		instanceCount = lastInstance - firstIstance;

		vkCmdDrawIndexed(
			cmd,
			meshAlloc.indexCount,
			instanceCount,
			meshAlloc.firstIndex,
			meshAlloc.vertexOffset,
			firstIstance
		);


		firstIstance = lastInstance;
	}




	//debugLayer.update();





	vkCmdEndRendering(cmd);

	// Transition to present layout
	barrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	barrier.dstAccessMask = 0;
	barrier.oldLayout	  = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	barrier.newLayout	  = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	vkCmdPipelineBarrier(
		cmd, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0,
		nullptr, 1, &barrier
	);

	vkEndCommandBuffer(cmd);

	VkSubmitInfo submitInfo {};
	submitInfo.sType				  = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount	  = 1;
	submitInfo.pWaitSemaphores		  = &static_cast<const VkSemaphore&>(mImageAvailableSemaphore[currentFrame]);
	submitInfo.pWaitDstStageMask	  = waitStages;
	submitInfo.commandBufferCount	  = 1;
	submitInfo.pCommandBuffers		  = &cmd;
	submitInfo.signalSemaphoreCount	  = 1;
	submitInfo.pSignalSemaphores	  = &static_cast<const VkSemaphore&>(mRenderFinishedSemaphore[imageIndex]);

	vkQueueSubmit(mGraphicsQueue, 1, &submitInfo, mInFlightFence[currentFrame]);

	VkPresentInfoKHR presentInfo {};
	presentInfo.sType			   = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores	   = &static_cast<const VkSemaphore&>(mRenderFinishedSemaphore[imageIndex]);
	VkSwapchainKHR swapchains	   = mSwapchain;
	presentInfo.swapchainCount	   = 1;
	presentInfo.pSwapchains		   = &swapchains;
	presentInfo.pImageIndices	   = &imageIndex;

	vkQueuePresentKHR(mPresentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % nFramesInFlight;
}

} // namespace Lithe
