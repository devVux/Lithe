#pragma once
#include "pch.h"

#include <LLGL/LLGL.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/CommandBufferFlags.h>

#include "Lithe/Core/Window.h"


namespace Lithe {

	class Renderer {

		public:

			void init(std::shared_ptr<LLGL::Surface> surface, const LLGL::RenderSystemDescriptor& descriptor);

			void draw();


		private:

			LLGL::RenderSystemPtr pRenderer			{ nullptr };
			LLGL::SwapChain* pSwapChain				{ nullptr };
			LLGL::CommandQueue* pCommandQueue		{ nullptr };
			LLGL::CommandBuffer* pCommands			{ nullptr };
			LLGL::PipelineState* pPipeline			{ nullptr };
			LLGL::Buffer* vertexBuffer				{ nullptr };

	};


}