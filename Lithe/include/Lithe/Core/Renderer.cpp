#include "pch.h"
#include "Renderer.h"

#include "Lithe/Core/Log.h"
#include "Lithe/Utils/Utils.h"

#include <LLGL/Window.h>
#include <LLGL/Utils/VertexFormat.h>

#include <glm/ext.hpp>

namespace Lithe {

	namespace Misc {
		struct Vertex {
			float position[2];
			uint8_t color[4];
		};

	}

	void Renderer::init(SharedPtr<LLGL::Surface> surface, const LLGL::RenderSystemDescriptor& descriptor) {
		{
			pRenderer = LLGL::RenderSystem::Load(descriptor);
			if (!pRenderer)
				Lithe::Log::FATAL("Could not load renderer {}", descriptor.moduleName);
		}
		
		{
			LLGL::SwapChainDescriptor swapChainDesc;
			swapChainDesc.resolution = { 800, 600 };
			swapChainDesc.samples = 1;
			pSwapChain = pRenderer->CreateSwapChain(swapChainDesc, surface);
		}

		LLGL::Shader* vertexShader;
		LLGL::Shader* fragmentShader;
		{
			const Misc::Vertex vertices[] = {
				{ {  0.0f,   0.5f }, { 255, 0,		0, 1 } },
				{ { -0.5f,  -0.5f }, { 0,	255,	0, 1 } },
				{ {  0.5f,  -0.5f }, { 0,	0,		255, 1 } },
			};


			LLGL::VertexFormat vertexFormat;
			vertexFormat.AppendAttribute({ "position", LLGL::Format::RG32Float });
			vertexFormat.AppendAttribute({ "color", LLGL::Format::RGBA8UNorm });

			LLGL::BufferDescriptor bufferDesc;
			bufferDesc.size = sizeof(vertices);
			bufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
			bufferDesc.vertexAttribs = vertexFormat.attributes;
			pVertexBuffer = pRenderer->CreateBuffer(bufferDesc, vertices);
		

			LLGL::ShaderDescriptor vertexDesc;
			vertexDesc.type = LLGL::ShaderType::Vertex;
			vertexDesc.sourceType = LLGL::ShaderSourceType::CodeFile;

#if (__APPLE__)
			vertexDesc.source = SHADERS_DIR"default.msl.vert";
#else
			vertexDesc.source = SHADERS_DIR"default.glsl.vert";
#endif

			vertexDesc.entryPoint = VERTEX_ENTRY_POINT;
			vertexDesc.profile = VERTEX_PROFILE;
			vertexDesc.vertex.inputAttribs = vertexFormat.attributes;
			vertexShader = pRenderer->CreateShader(vertexDesc);

			LLGL::ShaderDescriptor fragmentDesc;
			fragmentDesc.type = LLGL::ShaderType::Fragment;
			fragmentDesc.sourceType = LLGL::ShaderSourceType::CodeFile;

			#if (__APPLE__)
				fragmentDesc.source = SHADERS_DIR"default.msl.frag";
			#else
				fragmentDesc.source = SHADERS_DIR"default.glsl.frag";
			#endif

			fragmentDesc.entryPoint = FRAGMENT_ENTRY_POINT;
			fragmentDesc.profile = FRAGMENT_PROFILE;
			fragmentShader = pRenderer->CreateShader(fragmentDesc);

			// Check for shader compilation error
			for (LLGL::Shader* shader : { vertexShader, fragmentShader })
				if (const LLGL::Report* report = shader->GetReport())
					Lithe::Log::ERR("Shader compilation report: {}", report->GetText());

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
			if (const LLGL::Report* report = pPipeline->GetReport())
				Lithe::Log::ERR("Pipeline creation report: {}", report->GetText());

		}

		{
			pCommandQueue = pRenderer->GetCommandQueue();
			pCommands = pRenderer->CreateCommandBuffer();
		}


		{
			LLGL::BufferDescriptor bufferDesc;
			bufferDesc.size = sizeof(glm::mat4);
			bufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
			bufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
			pCameraBuffer = pRenderer->CreateBuffer(bufferDesc);
		}


		{
			LLGL::PipelineLayoutDescriptor layoutDesc;
			layoutDesc.heapBindings = {
				LLGL::BindingDescriptor{ "cameraBuffer", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 0 },
			};
			layoutDesc.uniforms = {
				LLGL::UniformDescriptor{ "uViewProjection", LLGL::UniformType::Float4x4 }
			};

			auto pipelineLayout = pRenderer->CreatePipelineLayout(layoutDesc);

			LLGL::ResourceViewDescriptor resourceViews[] = { pCameraBuffer };
			pResourceHeap = pRenderer->CreateResourceHeap(pipelineLayout, resourceViews);
		}


	}

	void Renderer::draw(const Lithe::Camera* camera) {
		if (!camera) {
			Log::WARN("No active camera; skipping rendering");
			return;
		}

		pRenderer->WriteBuffer(*pCameraBuffer, 0, glm::value_ptr(camera->viewProjection()), sizeof(camera->viewProjection()));

		pCommands->Begin();
		pCommands->SetViewport(pSwapChain->GetResolution());
		pCommands->BeginRenderPass(*pSwapChain);
		{
			pCommands->Clear(LLGL::ClearFlags::Color, { 0.1f, 0.1f, 0.1f, 1.0f });
			pCommands->SetPipelineState(*pPipeline);

			pCommands->SetResourceHeap(*pResourceHeap);

			pCommands->SetVertexBuffer(*pVertexBuffer);
			pCommands->Draw(3, 0);
		}
		pCommands->EndRenderPass();
		pCommands->End();
		pCommandQueue->Submit(*pCommands);
		pSwapChain->Present();
	
	}


}