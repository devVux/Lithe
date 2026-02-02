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

Buffer Allocator::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, Mapped doMap) {
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

void Allocator::cleanup() noexcept {
	if (!mAllocator)
		return;

	for (auto& buffer : mBuffers)
		vmaDestroyBuffer(mAllocator, buffer.handle, buffer.allocation);

	if (mAllocator) {
		vmaDestroyAllocator(mAllocator);
	}
}

}