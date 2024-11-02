
#include "pch.h"
#include "EntryPoint.h"

#include <LLGL/RenderSystem.h>
#include <LLGL/LLGL.h>
#include <LLGL/Window.h>
#include <LLGL/WindowFlags.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/Utils/TypeNames.h>
#include <iostream>



struct Vertex {
	float position[2];
};

int main() {
	LLGL::RenderSystemPtr pRenderer = nullptr;
	LLGL::SwapChain* pSwapChain = nullptr;
	LLGL::CommandQueue* pCommandQueue = nullptr;
	LLGL::CommandBuffer* pCommands = nullptr;
	LLGL::PipelineState* pPipeline = nullptr;
	LLGL::Buffer* vertexBuffer = nullptr;

	try {
		LLGL::RenderSystemDescriptor rendererDesc;
		rendererDesc.moduleName = "Metal";
		pRenderer = LLGL::RenderSystem::Load(rendererDesc);
		if (!pRenderer)
			throw std::runtime_error("Could not load renderer");

		LLGL::SwapChainDescriptor swapChainDesc;
		swapChainDesc.resolution = { 800, 600 };
		pSwapChain = pRenderer->CreateSwapChain(swapChainDesc);

		const Vertex vertices[] = {
			{ {  0.0f,  0.5f } },
			{ { -0.5f, -0.5f } },
			{ {  0.5f, -0.5f } },
		};

		LLGL::BufferDescriptor bufferDesc;
		bufferDesc.size = sizeof(vertices);
		bufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
		vertexBuffer = pRenderer->CreateBuffer(bufferDesc, vertices);

		const char* vertexShaderSource = R"(
			#include <metal_stdlib>
			using namespace metal;

			struct VertexIn {
				float2 position [[attribute(0)]];
			};

			struct VertexOut {
				float4 position [[position]];
			};

			vertex VertexOut vertex_main(VertexIn in [[stage_in]]) {
				VertexOut out;
				out.position = float4(in.position, 0.0, 1.0);
				return out;
			}
		)";

		const char* fragmentShaderSource = R"(
			#include <metal_stdlib>
			using namespace metal;

			struct FragmentOut {
				float4 fragColor [[color(0)]];
			};

			fragment FragmentOut fragment_main() {
				FragmentOut out;
				out.fragColor = float4(1.0, 0.4, 1.0, 1.0);  // Set a fixed pink color
				return out;
			}
		)";

		LLGL::VertexFormat vertexFormat;
		vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });

		LLGL::ShaderDescriptor vertexDesc;
		vertexDesc.sourceType = LLGL::ShaderSourceType::CodeString;
		vertexDesc.source = vertexShaderSource;
		vertexDesc.entryPoint = "vertex_main";
		vertexDesc.profile = "1.1";
		vertexDesc.vertex.inputAttribs = vertexFormat.attributes;
		LLGL::Shader* vertexShader = pRenderer->CreateShader(vertexDesc);

		LLGL::ShaderDescriptor fragmentDesc;
		fragmentDesc.sourceType = LLGL::ShaderSourceType::CodeString;
		fragmentDesc.source = fragmentShaderSource;
		fragmentDesc.entryPoint = "fragment_main";
		fragmentDesc.profile = "1.1";
		LLGL::Shader* fragmentShader = pRenderer->CreateShader(fragmentDesc);

		for (LLGL::Shader* shader : { vertexShader, fragmentShader }) {
			if (const LLGL::Report* report = shader->GetReport()) {
				if (report->HasErrors()) {
					std::cout << report->GetText();
				}
			}
		}

		LLGL::GraphicsPipelineDescriptor pipelineDesc;
		pipelineDesc.vertexShader = vertexShader;
		pipelineDesc.fragmentShader = fragmentShader;
		pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
		pipelineDesc.renderPass = pSwapChain->GetRenderPass();
		pPipeline = pRenderer->CreatePipelineState(pipelineDesc);

		pCommandQueue = pRenderer->GetCommandQueue();
		pCommands = pRenderer->CreateCommandBuffer();

		LLGL::Window& window = LLGL::CastTo<LLGL::Window>(pSwapChain->GetSurface());

		while (!window.HasQuit()) {
			pCommands->Begin();
			pCommands->SetViewport(pSwapChain->GetResolution());
			pCommands->SetVertexBuffer(*vertexBuffer);

			pCommands->BeginRenderPass(*pSwapChain);
			{
				pCommands->Clear(LLGL::ClearFlags::Color, { 0.7f, 0.5f, 0.2f, 1.0f });
				pCommands->SetPipelineState(*pPipeline);
				pCommands->SetVertexBuffer(*vertexBuffer);
				pCommands->Draw(3, 0);
			}
			pCommands->EndRenderPass();
			pCommands->End();

			pCommandQueue->Submit(*pCommands);
			pSwapChain->Present();
			window.ProcessEvents();
		}

		delete vertexBuffer;
		delete vertexShader;
		delete fragmentShader;
		delete pPipeline;
		delete pCommands;
		delete pSwapChain;

	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		delete vertexBuffer;
		delete pPipeline;
		delete pCommands;
		delete pSwapChain;
		return -1;
	}

	return 0;
}
