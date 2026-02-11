#include "DebugLayer.hpp"

#include "Events/KeyEvents.hpp"
#include "Events/MouseEvents.hpp"
#include "Events/WindowEvents.hpp"
#include "ForwardDecls.hpp"

#include <X11/X.h>
#include <imgui.h>
#include <imgui_impl_vulkan.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

namespace Lithe {

namespace {

	ImGuiKey ToImGuiKey(Key k) {
		switch (k) {
			case Key::TAB:			 return ImGuiKey_Tab;
			case Key::LEFT:			 return ImGuiKey_LeftArrow;
			case Key::RIGHT:		 return ImGuiKey_RightArrow;
			case Key::UP:			 return ImGuiKey_UpArrow;
			case Key::DOWN:			 return ImGuiKey_DownArrow;
			case Key::PAGE_UP:		 return ImGuiKey_PageUp;
			case Key::PAGE_DOWN:	 return ImGuiKey_PageDown;
			case Key::HOME:			 return ImGuiKey_Home;
			case Key::END:			 return ImGuiKey_End;
			case Key::INSERT:		 return ImGuiKey_Insert;
			case Key::DEL:			 return ImGuiKey_Delete;
			case Key::BACKSPACE:	 return ImGuiKey_Backspace;
			case Key::SPACE:		 return ImGuiKey_Space;
			case Key::ENTER:		 return ImGuiKey_Enter;
			case Key::ESCAPE:		 return ImGuiKey_Escape;
			case Key::APOSTROPHE:	 return ImGuiKey_Apostrophe;
			case Key::COMMA:		 return ImGuiKey_Comma;
			case Key::MINUS:		 return ImGuiKey_Minus;
			case Key::PERIOD:		 return ImGuiKey_Period;
			case Key::SLASH:		 return ImGuiKey_Slash;
			case Key::SEMICOLON:	 return ImGuiKey_Semicolon;
			case Key::EQUAL:		 return ImGuiKey_Equal;
			case Key::LEFT_BRACKET:	 return ImGuiKey_LeftBracket;
			case Key::BACKSLASH:	 return ImGuiKey_Backslash;
			case Key::RIGHT_BRACKET: return ImGuiKey_RightBracket;
			case Key::GRAVE_ACCENT:	 return ImGuiKey_GraveAccent;

			case Key::CAPS_LOCK:	 return ImGuiKey_CapsLock;
			case Key::SCROLL_LOCK:	 return ImGuiKey_ScrollLock;
			case Key::NUM_LOCK:		 return ImGuiKey_NumLock;
			case Key::PRINT_SCREEN:	 return ImGuiKey_PrintScreen;
			case Key::PAUSE:		 return ImGuiKey_Pause;

			case Key::KP_0:			 return ImGuiKey_Keypad0;
			case Key::KP_1:			 return ImGuiKey_Keypad1;
			case Key::KP_2:			 return ImGuiKey_Keypad2;
			case Key::KP_3:			 return ImGuiKey_Keypad3;
			case Key::KP_4:			 return ImGuiKey_Keypad4;
			case Key::KP_5:			 return ImGuiKey_Keypad5;
			case Key::KP_6:			 return ImGuiKey_Keypad6;
			case Key::KP_7:			 return ImGuiKey_Keypad7;
			case Key::KP_8:			 return ImGuiKey_Keypad8;
			case Key::KP_9:			 return ImGuiKey_Keypad9;
			case Key::KP_DECIMAL:	 return ImGuiKey_KeypadDecimal;
			case Key::KP_DIVIDE:	 return ImGuiKey_KeypadDivide;
			case Key::KP_MULTIPLY:	 return ImGuiKey_KeypadMultiply;
			case Key::KP_SUBTRACT:	 return ImGuiKey_KeypadSubtract;
			case Key::KP_ADD:		 return ImGuiKey_KeypadAdd;
			case Key::KP_ENTER:		 return ImGuiKey_KeypadEnter;
			case Key::KP_EQUAL:		 return ImGuiKey_KeypadEqual;

			case Key::LEFT_SHIFT:	 return ImGuiKey_LeftShift;
			case Key::LEFT_CONTROL:	 return ImGuiKey_LeftCtrl;
			case Key::LEFT_ALT:		 return ImGuiKey_LeftAlt;
			case Key::LEFT_SUPER:	 return ImGuiKey_LeftSuper;
			case Key::RIGHT_SHIFT:	 return ImGuiKey_RightShift;
			case Key::RIGHT_CONTROL: return ImGuiKey_RightCtrl;
			case Key::RIGHT_ALT:	 return ImGuiKey_RightAlt;
			case Key::RIGHT_SUPER:	 return ImGuiKey_RightSuper;
			case Key::MENU:			 return ImGuiKey_Menu;
		}

		if (k >= Key::A && k <= Key::Z)
			return ImGuiKey(ImGuiKey_A + (int(k) - int(Key::A)));

		if (k >= Key::_0 && k <= Key::_9)
			return ImGuiKey(ImGuiKey_0 + (int(k) - int(Key::_0)));

		if (k >= Key::F1 && k <= Key::F24)
			return ImGuiKey(ImGuiKey_F1 + (int(k) - int(Key::F1)));

		return ImGuiKey_None;
	}

} // namespace

static ImGui_ImplVulkanH_Window g_ImGuiWindow;

void DebugLayer::init(
	EventDispatcher& dispatcher,
	VkInstance		 instance,
	VkPhysicalDevice physicalDevice,
	VkDevice		 device,
	VkQueue			 graphicsQueue,
	uint32_t		 queueFamily,
	LtFormat		 colorFormat,
	uint32_t		 minImageCount,
	uint32_t		 imageCount,
	Size			 windowSize
) {

	// Create descriptor pool for ImGui
	VkDescriptorPoolSize poolSizes[] = {
		{			   VK_DESCRIPTOR_TYPE_SAMPLER, 1'000},
		{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1'000},
		{		 VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1'000},
		{		 VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1'000},
		{	 VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1'000},
		{	 VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1'000},
		{		 VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1'000},
		{		 VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1'000},
		{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1'000},
		{VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1'000},
		{		 VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1'000}
	};

	VkDescriptorPoolCreateInfo poolInfo = {};

	poolInfo.sType		   = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags		   = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets	   = 1'000;
	poolInfo.poolSizeCount = std::size(poolSizes);
	poolInfo.pPoolSizes	   = poolSizes;

	VkDescriptorPool descriptorPool;
	vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io		 = ImGui::GetIO();
	io.DisplaySize.x = windowSize.width;
	io.DisplaySize.y = windowSize.height;

	using namespace MouseEvents;
	using namespace KeyEvents;
	using namespace WindowEvents;

	dispatcher.on<MouseMovedEvent>([&io](const MouseMovedEvent& e) {
		io.AddMousePosEvent(e.pos.x, e.pos.y);
		return io.WantCaptureMouse;
	});

	dispatcher.on<MouseButtonPressedEvent>([&io](const MouseButtonPressedEvent& e) {
		io.AddMouseButtonEvent((int) e.button, true);
		return io.WantCaptureMouse;
	});

	dispatcher.on<MouseButtonReleasedEvent>([&io](const MouseButtonReleasedEvent& e) {
		io.AddMouseButtonEvent((int) e.button, false);
		return io.WantCaptureMouse;
	});

	dispatcher.on<MouseWheelEvent>([&io](const MouseWheelEvent& e) {
		io.AddMouseWheelEvent(e.pos.x, e.pos.y);
		return io.WantCaptureMouse;
	});

	dispatcher.on<KeyPressedEvent>([&io](const KeyPressedEvent& e) {
		io.AddKeyEvent(ToImGuiKey(e.keyCode), true);
		return io.WantCaptureKeyboard;
	});

	dispatcher.on<KeyReleasedEvent>([&io](const KeyReleasedEvent& e) {
		io.AddKeyEvent(ToImGuiKey(e.keyCode), false);
		return io.WantCaptureKeyboard;
	});

	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance				   = instance;
	initInfo.PhysicalDevice			   = physicalDevice;
	initInfo.Device					   = device;
	initInfo.QueueFamily			   = queueFamily;
	initInfo.Queue					   = graphicsQueue;
	initInfo.DescriptorPool			   = descriptorPool;
	initInfo.MinImageCount			   = minImageCount;
	initInfo.ImageCount				   = imageCount;
	initInfo.MSAASamples			   = VK_SAMPLE_COUNT_1_BIT;
	initInfo.UseDynamicRendering	   = true;

	VkPipelineRenderingCreateInfoKHR pipelineRenderingInfo = {};
	pipelineRenderingInfo.sType							   = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
	pipelineRenderingInfo.colorAttachmentCount			   = 1;
	pipelineRenderingInfo.pColorAttachmentFormats		   = (VkFormat*) &colorFormat;

	initInfo.PipelineRenderingCreateInfo = pipelineRenderingInfo;
	ImGui_ImplVulkan_Init(&initInfo);

	mDescriptorPool = RAIIed<VkDescriptorPool>(descriptorPool, [device](auto pool) noexcept {
		if (pool)
			vkDestroyDescriptorPool(device, pool, nullptr);
	});
}

void DebugLayer::update(VkCommandBuffer cmd, const DebugInfo& info) noexcept {
	ImGui_ImplVulkan_NewFrame();
	ImGui::NewFrame();

	// Example debug window
	ImGui::Begin("Debug Info");
	ImGui::Text("FPS: %.1f", 1 / info.delta);
	ImGui::Text("Frame time: %f ms", info.delta);
	ImGui::Text("Draw calls: %d", info.drawCalls);

	if (ImGui::CollapsingHeader("Stats")) {
		ImGui::Text("Instances: %d", info.instanceCount);
		ImGui::Text("Triangles: %d", info.triangleCount);
		ImGui::Text("Vertices: %d", info.vertexCount);

		for (const auto& [key, value] : info.materialDrawCalls)
			ImGui::Text("[Material = %d] draw calls: %d", key, value);

		for (const auto& [key, value] : info.meshDrawCalls)
			ImGui::Text("[Mesh = %d] draw calls: %d", key, value);
	}

	ImGui::End();

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}

DebugLayer::~DebugLayer() noexcept {
	ImGui_ImplVulkan_Shutdown();
	ImGui::DestroyContext();
}

} // namespace Lithe
