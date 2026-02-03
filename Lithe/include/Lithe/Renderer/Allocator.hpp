#pragma once

#include "ForwardDecls.hpp"

#include <vector>

namespace Lithe {

enum class Mapped: bool { 
	No = false,
	Yes = true
};

struct Buffer {
	VkBuffer handle { VK_NULL_HANDLE };
	VmaAllocation allocation { VK_NULL_HANDLE };
	void* mapped { nullptr };
};

struct Image {
	VkImage handle { VK_NULL_HANDLE };
	VmaAllocation allocation { VK_NULL_HANDLE };
	void* mapped { nullptr };
};

class Allocator {

	public:

		Allocator() = default;
		Allocator(VkPhysicalDevice, VkDevice, VkInstance);
		Allocator(const Allocator&) = delete;
		Allocator(Allocator&& other) noexcept;
		Allocator& operator=(const Allocator&) = delete;
		Allocator& operator=(Allocator&& other) noexcept;
		~Allocator() {
			cleanup();
		}

		Buffer createBuffer(VkDeviceSize, VkBufferUsageFlags, Mapped) noexcept;
		//Buffer createImage(VkDeviceSize, VkBufferUsageFlags, Mapped);
		Image createDepthImage(uint32_t width, uint32_t height, uint32_t depth = 1) noexcept;


	private:

		void cleanup() noexcept;

	private:
	
		VmaAllocator mAllocator { VK_NULL_HANDLE };
		std::vector<Buffer> mBuffers;
		std::vector<Image> mImages;

};

}