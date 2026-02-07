
#include "Log.hpp"
#include "SurfaceFactory.hpp"
#include "RenderSystem.hpp"

#include <algorithm>
#include <cassert>
#include <expected>
#include <fstream>
#include <optional>
#include <set>
#include <vector>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Lithe {

	
struct InitContext {
	VkDebugUtilsMessengerEXT debugMessenger {nullptr};
	VkSurfaceFormatKHR		 surfaceFormat;
	VkExtent2D				 extent;
};


namespace Validation {


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

	std::expected<std::pair<VkInstance, InitContext>, E> createInstance(
		const std::set<Extension>&										extensions,
		const std::set<std::string>&									layers,
		bool															hasValidationLayer = false
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

		InitContext ctx;
		VkInstance instance;
		if (!supportsLayers(layers) || vkCreateInstance(&instanceInfo, nullptr, &instance) != VK_SUCCESS)
			return std::unexpected {E::Unknown};

		if (hasValidationLayer)
			if (createDebugUtilsMessenger(instance, &debugInfo, nullptr, &ctx.debugMessenger) != VK_SUCCESS)
				return std::unexpected {E::Unknown};

		return std::make_pair(instance, ctx);
	}

} // namespace

namespace Device {

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

	auto enumeratePhysicalDevices(VkInstance instance) -> std::expected<std::vector<VkPhysicalDevice>, E> {
		uint32_t count = 0;
		vkEnumeratePhysicalDevices(instance, &count, nullptr);
		if (count == 0)
			return std::unexpected(E::Unknown);

		std::vector<VkPhysicalDevice> devices(count);
		vkEnumeratePhysicalDevices(instance, &count, devices.data());
		return devices;
	}

	auto
	selectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices, VkSurfaceKHR surface, uint32_t requiredQueues)
		-> std::expected<VkPhysicalDevice, E> {
		for (auto device : devices) {
			QueueFamilyIndices indices = findQueueFamilies(device, surface);
			if (!indices.covers(requiredQueues, surface != VK_NULL_HANDLE))
				continue;
			if (rateDevice(device) > 0)
				return device;
		}
		return std::unexpected(E::Unknown);
	}

	std::expected<VkDevice, E> create(VkPhysicalDevice physicalDevice, QueueFamilyIndices indices) {
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

		VkPhysicalDeviceVulkan12Features features {
			.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
			.pNext = (void*) &dynamicRenderingFeature,
			.shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
			.descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
			.descriptorBindingPartiallyBound = VK_TRUE,
			.runtimeDescriptorArray = VK_TRUE,
		};


		VkDeviceCreateInfo createInfo {
			.sType					 = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
			.pNext					 = &features,
			.queueCreateInfoCount	 = static_cast<uint32_t>(queueCreateInfos.size()),
			.pQueueCreateInfos		 = queueCreateInfos.data(),
			.enabledExtensionCount	 = 1,
			.ppEnabledExtensionNames = deviceExtensions,
		};

		VkDevice device;
		if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
			return std::unexpected {E::Unknown};

		return device;
	}

} // namespace

namespace Swapchain {

	struct SwapChainSupportDetails {
		VkSurfaceCapabilitiesKHR		capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR>	presentModes;
	};

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
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

	struct SwapchainCreationInfo {
		VkSwapchainKHR	   swapchain;
		VkSurfaceFormatKHR surfaceFormat;
		VkPresentModeKHR   presentMode;
		VkExtent2D		   extent;
	};

	std::expected<SwapchainCreationInfo, E> create(
		VkPhysicalDevice   physicalDevice,
		VkDevice		   device,
		VkSurfaceKHR	   surface,
		VkExtent2D		   requestedSize,
		QueueFamilyIndices indices
	) {
		SwapChainSupportDetails swapChainSupport = querySwapchainSupport(physicalDevice, surface);
		VkSurfaceFormatKHR		surfaceFormat	 = chooseSwapSurfaceFormat(swapChainSupport.formats);
		VkPresentModeKHR		presentMode		 = chooseSwapPresentMode(swapChainSupport.presentModes);
		VkExtent2D				extent			 = chooseSwapExtent(swapChainSupport.capabilities, requestedSize);

		uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
		if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
			imageCount = swapChainSupport.capabilities.maxImageCount;

		VkSwapchainCreateInfoKHR createInfo {};
		createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface			= surface;
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
		if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS)
			return std::unexpected {E::Unknown};

		return SwapchainCreationInfo {swapchain, surfaceFormat, presentMode, extent};
	}

} // namespace

namespace Pipeline {

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

	std::tuple<std::vector<VkDescriptorSetLayout>, std::expected<VkPipelineLayout, E>, std::expected<VkPipeline, E>>
	createPipeline(VkDevice device, VkExtent2D extent, VkFormat colorAttachmentFormat) {
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
		pipelineRenderingInfo.pColorAttachmentFormats		= &colorAttachmentFormat;
		pipelineRenderingInfo.depthAttachmentFormat			= VK_FORMAT_D32_SFLOAT;

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
		bindingDescription.stride						   = sizeof(float) * 8; // vec3 + vec3 + vec2
		bindingDescription.inputRate					   = VK_VERTEX_INPUT_RATE_VERTEX;

		std::vector<VkVertexInputAttributeDescription> attributeDescriptions = {
			VkVertexInputAttributeDescription {
				.location = 0,
				.binding = 0,
				.format	= VK_FORMAT_R32G32B32_SFLOAT,
				.offset = 0,
			},
			VkVertexInputAttributeDescription {
				.location = 1,
				.binding = 0,
				.format	= VK_FORMAT_R32G32B32_SFLOAT,
				.offset	  = sizeof(float) * 3,
			},
			VkVertexInputAttributeDescription {
				.location = 2,
				.binding = 0,
				.format	= VK_FORMAT_R32G32_SFLOAT,
				.offset	  = sizeof(float) * 6,
			}
		};


		VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
		vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount	= 1;
		vertexInputInfo.pVertexBindingDescriptions		= &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount	= static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
		inputAssembly.sType					 = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology				 = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		VkPipelineViewportStateCreateInfo viewportState = {};
		viewportState.sType								= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount						= 1;
		viewportState.scissorCount						= 1;

		VkPipelineRasterizationStateCreateInfo rasterizer = {};
		rasterizer.sType								  = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable						  = VK_FALSE;
		rasterizer.rasterizerDiscardEnable				  = VK_FALSE;
		rasterizer.polygonMode							  = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth							  = 1.0f;
		rasterizer.cullMode								  = VK_CULL_MODE_BACK_BIT;
		rasterizer.frontFace							  = VK_FRONT_FACE_COUNTER_CLOCKWISE;
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


		// set 0
		std::vector<VkDescriptorSetLayoutBinding> set0Bindings {
			{
				.binding			= 0,
				.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				.descriptorCount	= 1,
				.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT,
				.pImmutableSamplers = nullptr,
			}
		};

		// set 1
		std::vector<VkDescriptorSetLayoutBinding> set1Bindings {
			{
				.binding			= 0,
				.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLER,
				.descriptorCount	= 1,
				.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = nullptr,
			},
			{
				.binding			= 1,
				.descriptorType		= VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
				.descriptorCount	= 10,
				.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = nullptr,
			},
			{
				.binding			= 2,
				.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount	= 1,
				.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT,
				.pImmutableSamplers = nullptr,
			}
		};
		

		// set 2
		std::vector<VkDescriptorSetLayoutBinding> set2Bindings {
			{
				.binding			= 0,
				.descriptorType		= VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
				.descriptorCount	= 1,
				.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT,
				.pImmutableSamplers = nullptr,
			}
		};
		
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts(3);



		VkDescriptorSetLayoutCreateInfo layoutInfo {};
		layoutInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = static_cast<uint32_t>(set0Bindings.size());
		layoutInfo.pBindings	= set0Bindings.data();
		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[0]);


		// For bindless textures
		VkDescriptorSetLayoutBindingFlagsCreateInfoEXT flagsInfo{};
		flagsInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
		std::vector<VkDescriptorBindingFlags> bindingFlags = {
			0,
			VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT,
			0
		};
		flagsInfo.bindingCount = static_cast<uint32_t>(bindingFlags.size());
		flagsInfo.pBindingFlags = bindingFlags.data();

		layoutInfo.flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT;
		layoutInfo.pNext = &flagsInfo;
		layoutInfo.bindingCount = static_cast<uint32_t>(set1Bindings.size());
		layoutInfo.pBindings = set1Bindings.data();
		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[1]);


		layoutInfo.pNext = nullptr;
		layoutInfo.bindingCount = static_cast<uint32_t>(set2Bindings.size());
		layoutInfo.pBindings	= set2Bindings.data();
		vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &descriptorSetLayouts[2]);



		VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
		pipelineLayoutInfo.sType					  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount			  = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts				  = descriptorSetLayouts.data();
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

		return {descriptorSetLayouts, pipelineLayout, graphicsPipeline};
	}

} // namespace


bool QueueFamilyIndices::covers(uint32_t requiredQueues, bool needsPreset) const noexcept {
	if ((requiredQueues & VK_QUEUE_GRAPHICS_BIT) and not graphicsFamily.has_value())
		return false;
	if ((requiredQueues & VK_QUEUE_COMPUTE_BIT) and not computeFamily.has_value())
		return false;
	if ((requiredQueues & VK_QUEUE_TRANSFER_BIT) and not transferFamily.has_value())
		return false;
	if (needsPreset and not presentFamily.has_value())
		return false;

	return true;
}



}
