#include "RenderSystem.hpp"

#include "ForwardDecls.hpp"
#include "Log.hpp"
#include "SurfaceFactory.hpp"
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


constexpr size_t operator""_KB(unsigned long long v) {
	return v * 1'024;
}

constexpr size_t operator""_MB(unsigned long long v) {
	return v * 1'024 * 1'024;
}

constexpr size_t operator""_GB(unsigned long long v) {
	return v * 1'024 * 1'024 * 1'024;
}



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

bool RenderSystem::init(ISurface& surface, std::set<Extension> extensions) {

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

	auto result =
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




	mAllocator = Allocator(mPhysicalDevice, mDevice, mInstance);

	mVertexBuffer =
		mAllocator.createBuffer(1_MB, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Mapped::Yes);

	mIndexBuffer =
		mAllocator.createBuffer(1_MB, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Mapped::Yes);

	mUniformBuffers.resize(nFramesInFlight);
	for (size_t i = 0; i < nFramesInFlight; i++)
		mUniformBuffers[i] = mAllocator.createBuffer(1_MB, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, Mapped::Yes);



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
	}

	for (int i = 0; i < nFramesInFlight; i++) {
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



	VkDescriptorPoolSize poolSize {};
	poolSize.type			 = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize.descriptorCount = static_cast<uint32_t>(nFramesInFlight);

	VkDescriptorPoolCreateInfo poolInfo {};
	poolInfo.sType		   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = 1;
	poolInfo.pPoolSizes	   = &poolSize;
	poolInfo.maxSets	   = static_cast<uint32_t>(nFramesInFlight);

	VkDescriptorPool descriptorPool;
	assert(vkCreateDescriptorPool(mDevice, &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS);


	std::vector<VkDescriptorSetLayout> duplicateSetLayouts(nFramesInFlight, mDescriptorSetLayout);

	VkDescriptorSetAllocateInfo allocInfo {};
	allocInfo.sType				 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool	 = descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(duplicateSetLayouts.size());
	allocInfo.pSetLayouts		 = duplicateSetLayouts.data();


	mDescriptorSets.resize(nFramesInFlight);

	assert(vkAllocateDescriptorSets(mDevice, &allocInfo, mDescriptorSets.data()) == VK_SUCCESS);


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



struct Vertex {
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 uv;
};

bool RenderSystem::uploadStaticData(StaticRenderPacket& statics, IResourceCache& cache) noexcept {
	
	std::vector<Vertex> vertices(cache.countVertices());
	std::vector<uint32_t> indices(cache.countIndices());

	for (auto id : statics.meshes) {
		auto data = cache.getMesh(id);

		for (size_t i = 0; i < data.position.size(); ++i)
			vertices[i] = Vertex {
				.position = data.position[i],
				//.normal = data.normal[i],
				//.uv = data.uv[i],
			};


		indices = data.indices;

	}

	std::memcpy(mVertexBuffer.mapped, vertices.data(), vertices.size() * sizeof(Vertex));
	std::memcpy(mIndexBuffer.mapped, indices.data(), indices.size() * sizeof(uint32_t));

	return true;
}


void RenderSystem::render(DynamicRenderPacket& dynamics, IResourceCache& cache) {
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
	barrier.image							= mImages[imageIndex];
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
	colorAttachment.imageView				  = mImageViews.at(imageIndex);
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

	VkExtent2D size {800, 600};

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
	viewport.y = 0.0f;
	viewport.width = (float)size.width;
	viewport.height = (float)size.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D   scissor	= {
		   {			0,		   0},
		   {size.width, size.height}
	   };
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);



	std::memcpy(mUniformBuffers[currentFrame].mapped, &dynamics.camera, sizeof(glm::mat4));

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = mUniformBuffers[currentFrame].handle;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(glm::mat4);

	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = mDescriptorSets[currentFrame];
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;

	vkUpdateDescriptorSets(mDevice, 1, &descriptorWrite, 0, nullptr);



	vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[currentFrame], 0, nullptr);


	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(cmd, 0, 1, &mVertexBuffer.handle, offsets);
	vkCmdBindIndexBuffer(cmd, mIndexBuffer.handle, 0, VK_INDEX_TYPE_UINT32);

	vkCmdDrawIndexed(cmd, cache.countIndices(), 1, 0, 0, 0);




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
