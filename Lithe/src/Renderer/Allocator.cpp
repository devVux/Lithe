#include "Allocator.hpp"

#include <vulkan/vulkan.h>
#define VMA_IMPLEMENTATION
#include <vma/vk_mem_alloc.h>

namespace Lithe {


Allocator::Allocator(VkPhysicalDevice physicalDevice, VkDevice device, VkInstance instance) {
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = physicalDevice;
	allocatorInfo.device = device;
	allocatorInfo.instance = instance;
	allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_3;

	vmaCreateAllocator(&allocatorInfo, &mAllocator);
}

Allocator& Allocator::operator=(Allocator&& other) noexcept {
	if (this != &other) {
		cleanup();

		mAllocator = std::exchange(other.mAllocator, VK_NULL_HANDLE);
		mBuffers = std::move(other.mBuffers);
	}

	return *this;
}

Allocator::Allocator(Allocator&& other) noexcept :
	mAllocator(std::exchange(other.mAllocator, VK_NULL_HANDLE)),
	mBuffers(std::move(other.mBuffers)) {
}

Buffer Allocator::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, Mapped doMap) noexcept {
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	if (doMap == Mapped::Yes)
		allocInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;


	Buffer buffer;
	VmaAllocationInfo allocationInfo;
	vmaCreateBuffer(mAllocator, &bufferInfo, &allocInfo, &buffer.handle, &buffer.allocation, &allocationInfo);

	if (doMap == Mapped::Yes)
		buffer.mapped = allocationInfo.pMappedData;

	mBuffers.push_back(buffer);

	return buffer;
}

Image Allocator::createImage(uint32_t w, uint32_t h, VkFormat format, VkBufferUsageFlags usage) noexcept {
	VkImageCreateInfo info{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = { w, h, 1},
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImage image;
	VmaAllocation imageAllocation;

	vmaCreateImage(mAllocator, &info, &allocInfo, &image, &imageAllocation, nullptr);

	return mImages.emplace_back(Image{
		.handle = image,
		.allocation = imageAllocation
	});
}

Image Allocator::createDepthImage(uint32_t width, uint32_t height, uint32_t depth) noexcept {

	VkImageCreateInfo depthInfo{
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = VK_FORMAT_D32_SFLOAT,
		.extent = { width, height, depth },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
	};

	VmaAllocationCreateInfo allocInfo{};
	allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkImage depthImage;
	VmaAllocation depthAllocation;

	vmaCreateImage(mAllocator, &depthInfo, &allocInfo, &depthImage, &depthAllocation, nullptr);

	return mImages.emplace_back(Image {
		.handle = depthImage,
		.allocation = depthAllocation 
	});
}

void Allocator::cleanup() noexcept {
	if (!mAllocator)
		return;

	for (auto& buffer : mBuffers)
		vmaDestroyBuffer(mAllocator, buffer.handle, buffer.allocation);

	for (auto& image : mImages)
		vmaDestroyImage(mAllocator, image.handle, image.allocation);

	if (mAllocator)
		vmaDestroyAllocator(mAllocator);
}

}