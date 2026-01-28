#pragma once

#include <functional>
#include <utility>

namespace {

#ifndef LT_USE_64_BIT_PTR_DEFINES
#if defined(__LP64__) || defined(_WIN64) || (defined(__x86_64__) && !defined(__ILP32__)) || defined(_M_X64) || \
	defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__) ||                   \
	(defined(__riscv) && __riscv_xlen == 64)
#define LT_USE_64_BIT_PTR_DEFINES 1
#else
#define LT_USE_64_BIT_PTR_DEFINES 0
#endif
#endif

#define LT_DEFINE_HANDLE(object) typedef struct object##_T* object;

#ifndef LT_DEFINE_NON_DISPATCHABLE_HANDLE
#if (LT_USE_64_BIT_PTR_DEFINES == 1)
#define LT_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T* object;
#else
#define LT_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif
#endif

LT_DEFINE_HANDLE(VkInstance)
LT_DEFINE_HANDLE(VkPhysicalDevice)
LT_DEFINE_HANDLE(VkDevice)
LT_DEFINE_HANDLE(VkQueue)
LT_DEFINE_HANDLE(VkCommandBuffer)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkSwapchainKHR)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkBuffer)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkImage)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkSemaphore)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkFence)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkDeviceMemory)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkQueryPool)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkImageView)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkCommandPool)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkRenderPass)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkFramebuffer)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkEvent)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkBufferView)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkShaderModule)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineCache)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipelineLayout)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkPipeline)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSetLayout)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkSampler)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorSet)
LT_DEFINE_NON_DISPATCHABLE_HANDLE(VkDescriptorPool)

template<typename T, typename Deleter = std::function<void(T)>>
struct RAIIed {
	T		t {};
	Deleter deleter {};

	RAIIed() = default;

	RAIIed(T t_, Deleter d = [](T) { }) : t(std::move(t_)), deleter(std::move(d)) { }

	RAIIed(const RAIIed&)	  = delete;
	RAIIed(RAIIed&&) noexcept = default;

	~RAIIed() noexcept {
		if (deleter)
			deleter(t);
	}

	RAIIed& operator=(const RAIIed&) = delete;
	RAIIed& operator=(RAIIed&&)		 = default;

	operator const T&() const noexcept { return t; }
};

} // namespace Lithe
