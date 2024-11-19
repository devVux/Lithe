#include "pch.h"
#include "Renderer.h"

#include "Lithe/Core/Log.h"

#include <LLGL/Window.h>
#include <LLGL/Utils/VertexFormat.h>

namespace Lithe {

	namespace Misc {
		struct Vertex {
			float vertices[2];
		};

		const char* vertexShaderSource = R"(
            #version 420 core
            layout(location = 0) in vec2 position;
            void main() {
                gl_Position = vec4(position, 0.0, 1.0);
            }
        )";

		const char* fragmentShaderSource = R"(
            #version 420 core
            out vec4 fragColor;
            void main() {
                fragColor = vec4(1.0, 0.0, 0.0, 1.0);
            }
        )";
	}

	void Renderer::init(const LLGL::RenderSystemDescriptor& descriptor) {
		{
			pRenderer = LLGL::RenderSystem::Load(descriptor);
			if (!pRenderer)
				Lithe::Log::FATAL("Could not load renderer {}", descriptor.moduleName);
		}

		{
			LLGL::SwapChainDescriptor swapChainDesc;
			swapChainDesc.resolution = { 800, 600 };
			swapChainDesc.samples = 1;
			pSwapChain = pRenderer->CreateSwapChain(swapChainDesc);
		}


		{
			const Misc::Vertex vertices[] = {
				{ {  0.0f,   0.8f } },
				{ { -0.8f,  -0.8f } },
				{ {  0.8f,  -0.8f } }
			};

			LLGL::BufferDescriptor bufferDesc;
			bufferDesc.size = sizeof(vertices);
			bufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
			bufferDesc.vertexAttribs = { LLGL::VertexAttribute("position", LLGL::Format::RG32Float) };
			vertexBuffer = pRenderer->CreateBuffer(bufferDesc, vertices);
		}

		LLGL::Shader* vertexShader;
		LLGL::Shader* fragmentShader;
		{

			LLGL::VertexFormat vertexFormat;
			vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float, 0 });

			LLGL::ShaderDescriptor vertexDesc;
			vertexDesc.type = LLGL::ShaderType::Vertex;
			vertexDesc.sourceType = LLGL::ShaderSourceType::CodeString;
			vertexDesc.source = Misc::vertexShaderSource;
			vertexDesc.entryPoint = "";
			vertexDesc.profile = "";
			vertexDesc.vertex.inputAttribs = vertexFormat.attributes;
			vertexShader = pRenderer->CreateShader(vertexDesc);

			LLGL::ShaderDescriptor fragmentDesc;
			fragmentDesc.type = LLGL::ShaderType::Fragment;
			fragmentDesc.sourceType = LLGL::ShaderSourceType::CodeString;
			fragmentDesc.source = Misc::fragmentShaderSource;
			fragmentDesc.entryPoint = "";
			fragmentDesc.profile = "";
			fragmentShader = pRenderer->CreateShader(fragmentDesc);

			// Check for shader compilation errors
			for (LLGL::Shader* shader : { vertexShader, fragmentShader }) {
				if (const LLGL::Report* report = shader->GetReport()) {
					Lithe::Log::ERROR("Shader compilation report: ", report->GetText());
				}
			}

		}


		{

			LLGL::GraphicsPipelineDescriptor pipelineDesc;
			pipelineDesc.vertexShader = vertexShader;
			pipelineDesc.fragmentShader = fragmentShader;
			pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleList;
			pipelineDesc.renderPass = pSwapChain->GetRenderPass();
			pipelineDesc.rasterizer.cullMode = LLGL::CullMode::Front;

			pPipeline = pRenderer->CreatePipelineState(pipelineDesc);

			// Check for pipeline creation errors
			if (const LLGL::Report* report = pPipeline->GetReport()) {
				Lithe::Log::ERROR("Pipeline creation report: ", report->GetText());
			}

		}

		{
			pCommandQueue = pRenderer->GetCommandQueue();
			pCommands = pRenderer->CreateCommandBuffer();
		}

	}

	void Renderer::draw() {
		
		pCommands->Begin();
		pCommands->SetViewport(pSwapChain->GetResolution());
		pCommands->BeginRenderPass(*pSwapChain);
		{
			pCommands->Clear(LLGL::ClearFlags::Color, { 0.0f, 0.0f, 1.0f, 1.0f });
			pCommands->SetPipelineState(*pPipeline);
			pCommands->SetVertexBuffer(*vertexBuffer);
			pCommands->Draw(3, 0);
		}
		pCommands->EndRenderPass();
		pCommands->End();
		pCommandQueue->Submit(*pCommands);
		pSwapChain->Present();

	}


}