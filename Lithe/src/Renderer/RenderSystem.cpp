#include "RenderSystem.hpp"

#include "ForwardDecls.hpp"
#include "Log.hpp"

#include <algorithm>
#include <cassert>
#include <expected>
#include <fstream>
#include <optional>
#include <set>
#include <vector>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#ifdef LT_WIN32
#include <windows.h>
#include <vulkan/vulkan_win32.h>
#include <windows.h>
#endif

#ifdef LT_COCOA
#include <QuartzCore/CAMetalLayer.h>
#endif

#ifdef LT_X11
#include <X11/Xlib.h>
#include <vulkan/vulkan_xlib.h>
#endif

#ifdef LT_WAYLAND
#include <wayland-client.h>
#include <vulkan/vulkan_wayland.h>
#endif

struct Vertex {
	float pos[3];
	float color[4];
};

static int currentFrame	   = 0;
static int nFramesInFlight = 2;

namespace Lithe {

namespace {

	VkDebugUtilsMessengerEXT sDebugMessenger {nullptr};

	enum class Error {
		Unknown,
	};

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void*
	) {
		switch (messageSeverity) {
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT:
				LT_LOG_TRACE("Vulkan: {}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT:
				LT_LOG_INFO("Vulkan: {}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
				LT_LOG_WARN("Vulkan: {}", pCallbackData->pMessage);
				break;
			case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
				LT_LOG_ERROR("Vulkan: {}", pCallbackData->pMessage);
				break;
			default: LT_LOG_DEBUG("Vulkan: {}", pCallbackData->pMessage); break;
		}

		return VK_FALSE;
	}

	VkResult createDebugUtilsMessenger(
		VkInstance								  instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks*			  pAllocator,
		VkDebugUtilsMessengerEXT*				  pDebugMessenger
	) {
		auto func =
			(PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	void destroyDebugUtilsMessenger(
		VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator
	) {
		auto func =
			(PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

	bool satisfies(const std::set<std::string>& available, std::set<std::string> required) {
		for (auto& name : available)
			required.erase(name);

		return required.empty();
	}

	bool supportsLayers(const std::set<std::string>& layers) {
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		std::set<std::string> availableLayerNames;
		for (const auto& properties : availableLayers)
			availableLayerNames.emplace(properties.layerName);

		return satisfies(availableLayerNames, layers);
	}

	std::expected<VkInstance, Error> createInstance(
		const std::set<Extension>& extensions, const std::set<std::string>& layers, bool hasValidationLayer = false
	) {
		VkApplicationInfo appInfo {
			.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO,
			.pApplicationName	= "No name project", // might wanna set this as the project name on the
													 // editor or game
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
			.pEngineName		= "Lithe Engine",
			.engineVersion		= VK_MAKE_VERSION(1, 0, 0),
			.apiVersion			= VK_API_VERSION_1_3
		};

		std::vector<const char*> vector_layers_cstr;
		for (const auto& layer : layers)
			vector_layers_cstr.push_back(layer.c_str());

		std::vector<const char*> vector_extensions_cstr;
		for (const auto& ext : extensions)
			vector_extensions_cstr.push_back(ext.c_str());

		VkInstanceCreateInfo instanceInfo {
			.sType					 = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
			.pNext					 = nullptr,
			.flags					 = 0,
			.pApplicationInfo		 = &appInfo,
			.enabledLayerCount		 = static_cast<uint32_t>(vector_layers_cstr.size()),
			.ppEnabledLayerNames	 = vector_layers_cstr.data(),
			.enabledExtensionCount	 = static_cast<uint32_t>(vector_extensions_cstr.size()),
			.ppEnabledExtensionNames = vector_extensions_cstr.data()
		};

		VkDebugUtilsMessengerCreateInfoEXT debugInfo;
		if (hasValidationLayer) {

			debugInfo = VkDebugUtilsMessengerCreateInfoEXT {
				.sType			 = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
				.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
								   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
								   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
				.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
				.pfnUserCallback = debugCallback
			};

			instanceInfo.pNext = &debugInfo;
		}

		VkInstance instance;
		if (!supportsLayers(layers) || vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
			return std::unexpected {Error::Unknown};

		if (hasValidationLayer)
			if (createDebugUtilsMessenger(instance, &debugInfo, nullptr, &sDebugMessenger) != VK_SUCCESS)
				return std::unexpected {Error::Unknown};

		return {instance};
	}

} // namespace

namespace {

	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> computeFamily;
		std::optional<uint32_t> transferFamily;
		std::optional<uint32_t> presentFamily;

		bool covers(VkQueueFlags requiredQueues, bool needPreset) const noexcept {
			if ((requiredQueues & VK_QUEUE_GRAPHICS_BIT) and not graphicsFamily.has_value())
				return false;
			if ((requiredQueues & VK_QUEUE_COMPUTE_BIT) and not computeFamily.has_value())
				return false;
			if ((requiredQueues & VK_QUEUE_TRANSFER_BIT) and not transferFamily.has_value())
				return false;
			if (needPreset and not presentFamily.has_value())
				return false;

			return true;
		}
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
		QueueFamilyIndices indices;
		uint32_t		   count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
		std::vector<VkQueueFamilyProperties> families(count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &count, families.data());

		for (uint32_t i = 0; i < count; ++i) {
			VkQueueFlags flags = families[i].queueFlags;

			if (surface) {
				VkBool32 present = VK_FALSE;
				vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &present);

				// Prefer a queue family that supports both graphics and present
				if ((flags & VK_QUEUE_GRAPHICS_BIT) && present && !indices.graphicsFamily.has_value()) {
					indices.graphicsFamily = i;
					indices.presentFamily  = i;
				}

				// Fallback for separate present queue
				if (present && !indices.presentFamily.has_value())
					indices.presentFamily = i;
			}

			if ((flags & VK_QUEUE_GRAPHICS_BIT) && !indices.graphicsFamily.has_value())
				indices.graphicsFamily = i;
			if ((flags & VK_QUEUE_COMPUTE_BIT) && !indices.computeFamily.has_value())
				indices.computeFamily = i;
			if ((flags & VK_QUEUE_TRANSFER_BIT) && !indices.transferFamily.has_value())
				indices.transferFamily = i;
		}

		return indices;
	}

	// TODO:
	auto rateDevice(VkPhysicalDevice device) {
		return 1;
	}

	struct PhysicalDeviceBundle {
		VkPhysicalDevice   physicalDevice;
		QueueFamilyIndices indices;
	};

	std::vector<PhysicalDeviceBundle>
	suitableDevices(VkInstance instance, uint32_t requiredQueues, VkSurfaceKHR surface) {
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
		if (deviceCount == 0)
			return {};

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		std::vector<PhysicalDeviceBundle> suitable;

		for (auto* device : devices) {
			QueueFamilyIndices indices = findQueueFamilies(device, surface);

			if (!indices.covers(requiredQueues, surface != VK_NULL_HANDLE)) {
				VkPhysicalDeviceProperties props;
				vkGetPhysicalDeviceProperties(device, &props);
				LT_LOG_WARN("Found non compatible device {}", props.deviceName);

				continue;
			}

			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(device, &props);
			LT_LOG_TRACE("Found compatible device {}", props.deviceName);

			if (rateDevice(device) > 0)
				suitable.push_back({device, indices});
		};

		return suitable;
	}

	std::expected<VkDevice, Error> createDevice(PhysicalDeviceBundle bundle) {
		VkPhysicalDevice   physicalDevice = bundle.physicalDevice;
		QueueFamilyIndices indices		  = bundle.indices;

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
		std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies) {
			VkDeviceQueueCreateInfo queueCreateInfo {
				.sType			  = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
				.queueFamilyIndex = queueFamily,
				.queueCount		  = 1,
				.pQueuePriorities = &queuePriority
			};

			queueCreateInfos.push_back(queueCreateInfo);
		};

		const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_DYNAMIC_RENDERING_EXTENSION_NAME};
		LT_LOG_DEBUG("Vulkan device extensions:");
		for (const auto& ext : deviceExtensions)
			LT_LOG_DEBUG("  - {}", ext);

		constexpr VkPhysicalDeviceDynamicRenderingFeaturesKHR dynamicRenderingFeature {
			.sType			  = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
			.dynamicRendering = VK_TRUE,
		};

		VkDeviceCreateInfo createInfo {
			.sType					 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext					 = &dynamicRenderingFeature,
			.queueCreateInfoCount	 = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos		 = queueCreateInfos.data(),
			.enabledExtensionCount	 = 1,
			.ppEnabledExtensionNames = deviceExtensions
		};

		VkDevice device;
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
			return std::unexpected {Error::Unknown};

		return device;
	}

} // namespace

namespace {

	std::expected<VkSurfaceKHR, Error> createSurface(VkInstance instance, ISurface& surface) {
		VkSurfaceKHR vkSurface;
#ifdef LT_WIN32
		VkWin32SurfaceCreateInfoKHR info = {};
		info.sType						 = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		info.hinstance					 = GetModuleHandle(NULL);
		info.hwnd						 = surface.handle().handle.ptr;
		auto res						 = vkCreateWin32SurfaceKHR(instance, &info, nullptr, &vkSurface);
		if (res != VK_SUCCESS)
			return std::unexpected {Error::Unknown};
#elif defined(LT_X11)
		VkXlibSurfaceCreateInfoKHR info = {};
		info.sType						= VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
		info.dpy						= static_cast<Display*>(surface.native().display);
		info.window						= surface.native().handle.id;
		auto res						= vkCreateXlibSurfaceKHR(instance, &info, nullptr, &vkSurface);
		if (res != VK_SUCCESS)
			return std::unexpected {Error::Unknown};
#elif defined(LT_WAYLAND)
		VkWaylandSurfaceCreateInfoKHR info {
			.sType	 = VK_STRUCTURE_TYPE_WAYLAND_SURFACE_CREATE_INFO_KHR,
			.display = static_cast<wl_display*>(surface.native().display),
			.surface = static_cast<wl_surface*>(surface.native().handle.ptr),
		};

		auto res = vkCreateWaylandSurfaceKHR(instance, &info, nullptr, &vkSurface);
		if (res != VK_SUCCESS)
			return std::unexpected {Error::Unknown};
#endif

		return {vkSurface};
	}

} // namespace

namespace {
	static VkSurfaceFormatKHR surfaceFormat;

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR		capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR>	presentModes;
	};

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB &&
				availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}

		return availableFormats[0];
	}

	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes)
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR)
				return availablePresentMode;

		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, VkExtent2D size) {
		if (capabilities.currentExtent.width != UINT32_MAX)
			return capabilities.currentExtent;

		size.width	= std::clamp(size.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
		size.height = std::clamp(size.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

		return size;
	}

	SwapChainSupportDetails querySwapchainSupport(VkPhysicalDevice device, VkSurfaceKHR surface) {
		SwapChainSupportDetails details;

		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

		uint32_t formatCount;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

		if (formatCount != 0) {
			details.formats.resize(formatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
		}

		uint32_t presentModeCount;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

		if (presentModeCount != 0) {
			details.presentModes.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
		}

		return details;
	}

	std::expected<VkSwapchainKHR, Error> createSwapchain(
		VkPhysicalDevice   physicalDevice,
		VkDevice		   device,
		VkSurfaceKHR	   surface,
		VkExtent2D		   size,
		QueueFamilyIndices indices
	) {
		SwapChainSupportDetails swapChainSupport = querySwapchainSupport(physicalDevice, surface);

		surfaceFormat				 = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D		 extent		 = chooseSwapExtent(swapChainSupport.capabilities, size);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			imageCount = swapChainSupport.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo {};
		createInfo.sType   = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface;

		createInfo.minImageCount	= imageCount;
		createInfo.imageFormat		= surfaceFormat.format;
		createInfo.imageColorSpace	= surfaceFormat.colorSpace;
		createInfo.imageExtent		= extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

		if (indices.graphicsFamily != indices.presentFamily) {
			createInfo.imageSharingMode		 = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices	 = queueFamilyIndices;
		} else {
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		}

		createInfo.preTransform	  = swapChainSupport.capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		createInfo.presentMode	  = presentMode;
		createInfo.clipped		  = VK_TRUE;

		VkSwapchainKHR swapchain;
		VkResult	   res = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain);
		if (res != VK_SUCCESS)
			std::unexpected {Error::Unknown};

		return {swapchain};
	}

} // namespace

namespace {

	std::vector<uint32_t> loadBinary(const std::string& path) {
		LT_LOG_DEBUG(path);
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		size_t		  size = (size_t) file.tellg();
		file.seekg(0);

		std::vector<uint32_t> data(size / 4);
		file.read((char*) data.data(), size);
		return data;
	}

	VkShaderModule createShaderModule(VkDevice device, const std::string& filepath) {
		auto code = loadBinary(filepath);

		VkShaderModuleCreateInfo createInfo = {};
		createInfo.sType					= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize					= code.size() * sizeof(uint32_t);
		createInfo.pCode					= code.data();

		VkShaderModule shaderModule;
		vkCreateShaderModule(device, &createInfo, NULL, &shaderModule);
		return shaderModule;
	}

	std::tuple<std::expected<VkPipelineLayout, Error>, std::expected<VkPipeline, Error>>
	createPipeline(VkDevice device, VkExtent2D extent) {
		VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
		colorBlendAttachment.blendEnable						 = VK_FALSE;
		colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		VkPipelineColorBlendStateCreateInfo colorBlending = {};
		colorBlending.sType								  = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.attachmentCount					  = 1;
		colorBlending.pAttachments						  = &colorBlendAttachment;

		VkPipelineRenderingCreateInfo pipelineRenderingInfo = {};
		pipelineRenderingInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
		pipelineRenderingInfo.colorAttachmentCount			= 1;
		pipelineRenderingInfo.pColorAttachmentFormats		= &surfaceFormat.format;
		pipelineRenderingInfo.depthAttachmentFormat			= VK_FORMAT_UNDEFINED; // or VK_FORMAT_UNDEFINED if no depth

		auto vertShaderModule = createShaderModule(device, SHADERS_DIR "/vertex.spv");
		auto fragShaderModule = createShaderModule(device, SHADERS_DIR "/fragment.spv");

		VkPipelineShaderStageCreateInfo shaderStages[2] = {};
		shaderStages[0].sType							= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage							= VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module							= vertShaderModule;
		shaderStages[0].pName							= "main";

		shaderStages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = fragShaderModule;
		shaderStages[1].pName  = "main";

		VkVertexInputBindingDescription bindingDescription = {};
		bindingDescription.binding						   = 0;
		bindingDescription.stride						   = sizeof(float) * 7; // vec3 + vec4
		bindingDescription.inputRate					   = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription attributeDescriptions[2] = {};

		// position
		attributeDescriptions[0].binding  = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format	  = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[0].offset	  = 0;

		// color
		attributeDescriptions[1].binding  = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format	  = VK_FORMAT_R32G32B32A32_SFLOAT;
		attributeDescriptions[1].offset	  = sizeof(float) * 3;

		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount	= 1;
		vertexInputInfo.pVertexBindingDescriptions		= &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = 2;
		vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions;

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType					 = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology				 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		VkViewport viewport = {};
		viewport.x			= 0.0f;
		viewport.y			= (float) extent.height;
		viewport.width		= (float) extent.width;
		viewport.height		= -1 * (float) extent.height;
		viewport.minDepth	= 0.0f;
		viewport.maxDepth	= 1.0f;

		VkRect2D scissor = {};
		scissor.offset	 = {0, 0};
		scissor.extent	 = extent;

		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType								= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount						= 1;
		viewportState.pViewports						= &viewport;
		viewportState.scissorCount						= 1;
		viewportState.pScissors							= &scissor;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType								  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable						  = VK_FALSE;
		rasterizer.rasterizerDiscardEnable				  = VK_FALSE;
		rasterizer.polygonMode							  = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth							  = 1.0f;
		rasterizer.cullMode								  = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace							  = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.depthBiasEnable						  = VK_FALSE;

		VkPipelineMultisampleStateCreateInfo multisampling = {};
		multisampling.sType								   = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable				   = VK_FALSE;
		multisampling.rasterizationSamples				   = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencil = {};
		depthStencil.sType								   = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable					   = VK_TRUE;
		depthStencil.depthWriteEnable					   = VK_TRUE;
		depthStencil.depthCompareOp						   = VK_COMPARE_OP_LESS;
		depthStencil.depthBoundsTestEnable				   = VK_FALSE;
		depthStencil.stencilTestEnable					   = VK_FALSE;

		VkDynamicState dynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

		VkPipelineDynamicStateCreateInfo dynamicState = {};
		dynamicState.sType							  = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount				  = 2;
		dynamicState.pDynamicStates					  = dynamicStates;

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType					  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount			  = 0;
		pipelineLayoutInfo.pushConstantRangeCount	  = 0;

		VkPipelineLayout pipelineLayout;
		vkCreatePipelineLayout(device, &pipelineLayoutInfo, NULL, &pipelineLayout);

		VkGraphicsPipelineCreateInfo pipelineInfo = {};
		pipelineInfo.sType						  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext						  = &pipelineRenderingInfo;
		pipelineInfo.stageCount					  = 2;
		pipelineInfo.pStages					  = shaderStages;
		pipelineInfo.pVertexInputState			  = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState		  = &inputAssembly;
		pipelineInfo.pViewportState				  = &viewportState;
		pipelineInfo.pRasterizationState		  = &rasterizer;
		pipelineInfo.pMultisampleState			  = &multisampling;
		pipelineInfo.pDepthStencilState			  = &depthStencil;
		pipelineInfo.pColorBlendState			  = &colorBlending;
		pipelineInfo.pDynamicState				  = &dynamicState;
		pipelineInfo.layout						  = pipelineLayout;

		VkPipeline graphicsPipeline;
		vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, NULL, &graphicsPipeline);

		vkDestroyShaderModule(device, vertShaderModule, nullptr);
		vkDestroyShaderModule(device, fragShaderModule, nullptr);

		return {pipelineLayout, graphicsPipeline};
	}

} // namespace

namespace {
	std::expected<VkCommandPool, Error> createCommandPool(VkDevice device, QueueFamilyIndices indices) {

		VkCommandPoolCreateInfo poolInfo {};
		poolInfo.sType			  = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags			  = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = indices.graphicsFamily.value();

		VkCommandPool commandPool;
		if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS)
			return std::unexpected {Error::Unknown};

		return {commandPool};
	}

	std::expected<std::vector<VkCommandBuffer>, Error>
	createCommandBuffers(VkDevice device, VkCommandPool commandPool) {

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.resize(nFramesInFlight);

		VkCommandBufferAllocateInfo allocInfo {};
		allocInfo.sType				 = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool		 = commandPool;
		allocInfo.level				 = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			return std::unexpected {Error::Unknown};

		return {commandBuffers};
	}

} // namespace

uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeBits, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProps;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProps);

	for (uint32_t i = 0; i < memProps.memoryTypeCount; i++)
		if ((typeBits & (1u << i)) && (memProps.memoryTypes[i].propertyFlags & properties) == properties)
			return i;

	return UINT32_MAX;
}

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
	bool hasValidationLayer = !layers.empty() && satisfies(layers, {"VK_LAYER_KHRONOS_validation"});

	LT_LOG_DEBUG("Vulkan Extensions:");
	for (const auto& ext : extensions)
		LT_LOG_DEBUG("  - {}", ext);

	LT_LOG_DEBUG("Vulkan Layers:");
	for (const auto& layer : layers)
		LT_LOG_DEBUG("  - {}", layer);

	auto instance = createInstance(extensions, layers, hasValidationLayer);
	if (instance)
		mInstance = RAIIed<VkInstance>(*instance, [hasValidationLayer](VkInstance instance) noexcept {
			if (hasValidationLayer)
				destroyDebugUtilsMessenger(instance, sDebugMessenger, nullptr);

			if (instance)
				vkDestroyInstance(instance, nullptr);
		});
	else
		LT_LOG_FATAL("Could not create Vulkan instance");

	auto surface2 = createSurface(mInstance, surface);

	if (surface2)
		mSurface =
			RAIIed<VkSurfaceKHR>(*surface2, [instance = static_cast<VkInstance>(mInstance)](auto surface) noexcept {
				if (surface)
					vkDestroySurfaceKHR(instance, surface, nullptr);
			});
	else
		LT_LOG_FATAL("Could not create surface");

	auto physicalDevicesBundles = suitableDevices(mInstance, VK_QUEUE_GRAPHICS_BIT, mSurface);
	LT_LOG_TRACE("Found {} suitable devices", physicalDevicesBundles.size());
	assert(physicalDevicesBundles.size() != 0);

	auto selectedBundle = physicalDevicesBundles.at(0);
	LT_LOG_TRACE("Selecting first device");
	auto device = createDevice(selectedBundle);

	if (device)
		mDevice = RAIIed<VkDevice>(*device, [](auto device) noexcept {
			if (device)
				vkDestroyDevice(device, nullptr);
		});
	else
		LT_LOG_FATAL("Could not create device");

	vkGetDeviceQueue(mDevice, *selectedBundle.indices.graphicsFamily, 0, &mGraphicsQueue);
	vkGetDeviceQueue(mDevice, *selectedBundle.indices.presentFamily, 0, &mPresentQueue);

	VkExtent2D size {800, 600}; // surface size
	auto swapchain = createSwapchain(selectedBundle.physicalDevice, mDevice, mSurface, size, selectedBundle.indices);

	if (swapchain)
		mSwapchain =
			RAIIed<VkSwapchainKHR>(*swapchain, [device = static_cast<VkDevice>(mDevice)](auto swapchain) noexcept {
				if (swapchain)
					vkDestroySwapchainKHR(device, swapchain, nullptr);
			});
	else
		LT_LOG_FATAL("Could not create swapchain");

	uint32_t imageCount;
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, nullptr);
	mImages.resize(imageCount);
	vkGetSwapchainImagesKHR(mDevice, mSwapchain, &imageCount, mImages.data());

	auto support = querySwapchainSupport(selectedBundle.physicalDevice, mSurface);

	mImageViews.resize(imageCount);

	for (size_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo createInfo {};
		createInfo.sType						   = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image						   = mImages[i];
		createInfo.viewType						   = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format						   = surfaceFormat.format;
		createInfo.components.r					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a					   = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask	   = VK_IMAGE_ASPECT_COLOR_BIT;
		createInfo.subresourceRange.baseMipLevel   = 0;
		createInfo.subresourceRange.levelCount	   = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount	   = 1;

		VkImageView imageView;
		if (vkCreateImageView(mDevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
			LT_LOG_FATAL("Could not create image view {}", i);

		mImageViews[i] = RAIIed<VkImageView>(imageView, [device = static_cast<VkDevice>(mDevice)](auto view) noexcept {
			if (view)
				vkDestroyImageView(device, view, nullptr);
		});
	}

	auto [pipelineLayout, pipeline] = createPipeline(mDevice, size);

	if (pipelineLayout)
		mPipelineLayout = RAIIed<VkPipelineLayout>(
			*pipelineLayout, [device = static_cast<VkDevice>(mDevice)](auto pipelineLayout) noexcept {
				if (pipelineLayout)
					vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
			}
		);
	else
		LT_LOG_FATAL("Could not create pipeline layout");

	if (pipeline)
		mPipeline = RAIIed<VkPipeline>(*pipeline, [device = static_cast<VkDevice>(mDevice)](auto pipeline) noexcept {
			if (pipeline)
				vkDestroyPipeline(device, pipeline, nullptr);
		});
	else
		LT_LOG_FATAL("Could not create pipeline");

	auto commandPool = createCommandPool(mDevice, selectedBundle.indices);
	if (commandPool)
		mCommandPool =
			RAIIed<VkCommandPool>(*commandPool, [device = static_cast<VkDevice>(mDevice)](auto pool) noexcept {
				if (pool)
					vkDestroyCommandPool(device, pool, nullptr);
			});

	auto commandBuffers = createCommandBuffers(mDevice, mCommandPool);
	if (commandBuffers)
		mCommandBuffers = *commandBuffers;

	Vertex vertices[3] = {
		{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
		{ {0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}}
	};

	VkDeviceSize bufferSize = sizeof(vertices);

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType			  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size				  = bufferSize;
	bufferInfo.usage			  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	bufferInfo.sharingMode		  = VK_SHARING_MODE_EXCLUSIVE;

	VkBuffer vertexBuffer;
	vkCreateBuffer(mDevice, &bufferInfo, NULL, &vertexBuffer);
	mVertexBuffer = RAIIed<VkBuffer>(vertexBuffer, [device = static_cast<VkDevice>(mDevice)](auto buffer) noexcept {
		if (buffer)
			vkDestroyBuffer(device, buffer, nullptr);
	});

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(mDevice, vertexBuffer, &memReq);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType				   = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize	   = memReq.size;
	allocInfo.memoryTypeIndex	   = findMemoryType(
		 selectedBundle.physicalDevice, memReq.memoryTypeBits,
		 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	 );

	VkDeviceMemory vertexMemory;
	vkAllocateMemory(mDevice, &allocInfo, NULL, &vertexMemory);

	mMemory = RAIIed<VkDeviceMemory>(vertexMemory, [device = static_cast<VkDevice>(mDevice)](auto memory) noexcept {
		if (memory)
			vkFreeMemory(device, memory, nullptr);
	});

	vkBindBufferMemory(mDevice, vertexBuffer, vertexMemory, 0);

	void* data;
	vkMapMemory(mDevice, vertexMemory, 0, bufferSize, 0, &data);
	memcpy(data, vertices, bufferSize);
	vkUnmapMemory(mDevice, vertexMemory);

	VkSemaphoreCreateInfo semInfo = {};
	semInfo.sType				  = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (int i = 0; i < imageCount; i++) {
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

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType				= VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags				= VK_FENCE_CREATE_SIGNALED_BIT;

		VkFence inFlightFence;
		vkCreateFence(mDevice, &fenceInfo, NULL, &inFlightFence);
		mInFlightFence.push_back(
			RAIIed<VkFence>(inFlightFence, [device = static_cast<VkDevice>(mDevice)](auto fence) noexcept {
				if (fence)
					vkDestroyFence(device, fence, nullptr);
			})
		);
	}

	return true;
}

void RenderSystem::render() {
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

	// VkRenderingAttachmentInfo depthAttachment = {};
	// depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
	// depthAttachment.imageView = depthImageView;
	// depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
	// depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	// depthAttachment.clearValue.depthStencil = {1.0f, 0};

	VkExtent2D size {800, 600};

	VkRenderingInfo renderingInfo	   = {};
	renderingInfo.sType				   = VK_STRUCTURE_TYPE_RENDERING_INFO;
	renderingInfo.renderArea.offset	   = {0, 0};
	renderingInfo.renderArea.extent	   = size;
	renderingInfo.layerCount		   = 1;
	renderingInfo.colorAttachmentCount = 1;
	renderingInfo.pColorAttachments	   = &colorAttachment;
	// renderingInfo.pDepthAttachment = &depthAttachment;

	vkCmdBeginRendering(cmd, &renderingInfo);

	vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipeline);

	VkViewport viewport = {0.0f, 0.0f, (float) size.width, (float) size.height, 0.0f, 1.0f};
	VkRect2D   scissor	= {
		   {			0,		   0},
		   {size.width, size.height}
	   };
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);

	VkDeviceSize offsets[] = {0};
	vkCmdBindVertexBuffers(cmd, 0, 1, &static_cast<const VkBuffer&>(mVertexBuffer), offsets);

	vkCmdDraw(cmd, 3, 1, 0, 0);

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
