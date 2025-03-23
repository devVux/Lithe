#pragma once

#include "Utils.h"
#include "ForwardDecls.h"
#include "Window.h"
#include "Scene.h"

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
