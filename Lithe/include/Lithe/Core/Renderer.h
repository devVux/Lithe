#pragma once

#include "Utils.h"
#include "Window.h"
#include "Lithe/Scene/Scene.h"
#include "Lithe/Scene/Camera.h"
#include "ForwardDecls.h"

namespace Lithe {

	class Renderer {

		public:

			void init(SharedPtr<Lithe::Window> window);

			void draw(const Scene& scene, const Lithe::Camera* camera);


		private:

			LLGL::RenderSystem* pRenderer			{ nullptr };
			LLGL::SwapChain* pSwapChain				{ nullptr };
			LLGL::CommandQueue* pCommandQueue		{ nullptr };
			LLGL::CommandBuffer* pCommands			{ nullptr };
			LLGL::PipelineState* pPipeline			{ nullptr };
			LLGL::Buffer* pVertexBuffer				{ nullptr };
			LLGL::Buffer* pCameraBuffer				{ nullptr };
			LLGL::Buffer* pEntityBuffer				{ nullptr };
			LLGL::ResourceHeap* pResourceHeap		{ nullptr };

	};


}
