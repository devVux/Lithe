#include "pch.h"

#include "Log.h"
#include "Renderer.h"
#include "Window.h"
#include "RenderSystem.h"
#include "Utils.h"
#include "WindowWrapper.h"

#include <LLGL/LLGL.h>
#include <LLGL/Format.h>
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/PipelineState.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Utils/VertexFormat.h>
#include <LLGL/Shader.h>

#include <glm/ext.hpp>
#define GLM_ENABLE_EXPERIMENTAL 
#include <glm/gtx/string_cast.hpp>

namespace Lithe {

	#define MAX_VERTICES 100

	

	void Renderer::init(SharedPtr<Lithe::Window> window) {
		
		LLGL::RenderSystemDescriptor rendererDesc;
		rendererDesc.moduleName = LITHE_RENDERER;

		pRenderer = LLGL::RenderSystem::Load(rendererDesc).release();
		if (!pRenderer)
			Log::FATAL("Could not load renderer {}", rendererDesc.moduleName);

		
		{
			LLGL::SwapChainDescriptor swapChainDesc;
			swapChainDesc.resolution = { 800, 600 };
			swapChainDesc.samples = 1;
			pSwapChain = pRenderer->CreateSwapChain(swapChainDesc, makeShared<WindowWrapper>(window.get()));
		}

		LLGL::Shader* vertexShader;
		LLGL::Shader* fragmentShader;
		{
				
			LLGL::VertexFormat vertexFormat;
			vertexFormat.AppendAttribute({ "position", LLGL::Format::RGB32Float });

			LLGL::BufferDescriptor bufferDesc;
			bufferDesc.size = sizeof(OurRenderSystem::Vertex) * MAX_VERTICES * 4;
			bufferDesc.bindFlags = LLGL::BindFlags::VertexBuffer;
			bufferDesc.vertexAttribs = vertexFormat.attributes;
			bufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
			pVertexBuffer = pRenderer->CreateBuffer(bufferDesc, nullptr);
		

			LLGL::ShaderDescriptor vertexDesc;
			vertexDesc.type = LLGL::ShaderType::Vertex;
			vertexDesc.sourceType = LLGL::ShaderSourceType::CodeFile;

#if (__APPLE__)
			vertexDesc.source = SHADERS_DIR"default.msl.vert";
#else
			vertexDesc.source = SHADERS_DIR"default.glsl.vert";
#endif

			vertexDesc.entryPoint = "main";
			vertexDesc.profile = "version 420 core";
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

			fragmentDesc.entryPoint = "main";
			fragmentDesc.profile = "version 420 core";
			fragmentShader = pRenderer->CreateShader(fragmentDesc);

			// Check for shader compilation error
			for (LLGL::Shader* shader : { vertexShader, fragmentShader })
				if (const LLGL::Report* report = shader->GetReport())
					Log::ERR("Shader compilation report: {}", std::string(report->GetText()));

		}


		{

			LLGL::GraphicsPipelineDescriptor pipelineDesc;
			pipelineDesc.vertexShader = vertexShader;
			pipelineDesc.fragmentShader = fragmentShader;
			pipelineDesc.primitiveTopology = LLGL::PrimitiveTopology::TriangleStrip;
			pipelineDesc.renderPass = pSwapChain->GetRenderPass();
			pipelineDesc.rasterizer.cullMode = LLGL::CullMode::Front;

			pPipeline = pRenderer->CreatePipelineState(pipelineDesc);

			// Check for pipeline creation errors
			if (const LLGL::Report* report = pPipeline->GetReport())
				Log::ERR("Pipeline creation report: {}", std::string(report->GetText()));

		}

		
		pCommandQueue = pRenderer->GetCommandQueue();
		pCommands = pRenderer->CreateCommandBuffer();
		


		{
			LLGL::BufferDescriptor cameraBufferDesc;
			cameraBufferDesc.size = sizeof(glm::mat4);
			cameraBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
			cameraBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
			pCameraBuffer = pRenderer->CreateBuffer(cameraBufferDesc);
			
			LLGL::BufferDescriptor entityBufferDesc;
			entityBufferDesc.size = (sizeof(glm::mat4) + sizeof(glm::vec4)) * OurRenderSystem::MAX_ENTITIES;
			entityBufferDesc.bindFlags = LLGL::BindFlags::ConstantBuffer;
			entityBufferDesc.cpuAccessFlags = LLGL::CPUAccessFlags::Write;
			pEntityBuffer = pRenderer->CreateBuffer(entityBufferDesc);
		}


		{
			LLGL::PipelineLayoutDescriptor layoutDesc;
			layoutDesc.heapBindings = {
				LLGL::BindingDescriptor{ "CameraBuffer", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 0 },
				LLGL::BindingDescriptor{ "EntityBuffer", LLGL::ResourceType::Buffer, LLGL::BindFlags::ConstantBuffer, LLGL::StageFlags::VertexStage, 1 },
			};
			layoutDesc.uniforms = {
				LLGL::UniformDescriptor( "uViewProjection", LLGL::UniformType::Float4x4,	1   ),
				LLGL::UniformDescriptor( "uTransforms",		LLGL::UniformType::Float4x4,	OurRenderSystem::MAX_ENTITIES ),
				LLGL::UniformDescriptor( "uColors",			LLGL::UniformType::Float4,		OurRenderSystem::MAX_ENTITIES ),
			};

			auto pipelineLayout = pRenderer->CreatePipelineLayout(layoutDesc);

			LLGL::ResourceViewDescriptor resourceViews[] = { pCameraBuffer, pEntityBuffer };
			pResourceHeap = pRenderer->CreateResourceHeap(pipelineLayout, resourceViews);
		}


	}

	void Renderer::draw(const Scene& scene, const Lithe::Camera* camera) {
		if (!camera) {
			Log::WARN("No active camera; skipping rendering");
			return;
		}

		pRenderer->WriteBuffer(*pCameraBuffer, 0, glm::value_ptr(camera->viewProjection()), sizeof(glm::mat4));
		
		const auto& vertices = OurRenderSystem::buildVertices(scene);
		pRenderer->WriteBuffer(*pVertexBuffer, 0, vertices.data(), vertices.size() * sizeof(OurRenderSystem::Vertex));

		const auto& eb = OurRenderSystem::buildEntityBuffer(scene);
		pRenderer->WriteBuffer(*pEntityBuffer, 0, eb.buffer.data(), eb.buffer.size());


		pCommands->Begin();
		pCommands->SetViewport(pSwapChain->GetResolution());
		pCommands->BeginRenderPass(*pSwapChain);
		{
			pCommands->Clear(LLGL::ClearFlags::Color);
			pCommands->SetPipelineState(*pPipeline);
				
			pCommands->SetResourceHeap(*pResourceHeap);

			pCommands->SetVertexBuffer(*pVertexBuffer);
			pCommands->DrawInstanced(vertices.size(), 0, eb.entityCount);
		}
		pCommands->EndRenderPass();
		pCommands->End();
		pCommandQueue->Submit(*pCommands);
		pSwapChain->Present();
	
	}



}
