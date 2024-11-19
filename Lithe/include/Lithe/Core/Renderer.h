#pragma once
#include "pch.h"

#include <LLGL/LLGL.h>
#include <LLGL/RenderSystem.h>
#include <LLGL/CommandBufferFlags.h>


namespace Lithe {

	class Renderer {

		public:

			void init(const LLGL::RenderSystemDescriptor& descriptor);

			void draw();


		public:

			LLGL::Window& window() const { return LLGL::CastTo<LLGL::Window>(pSwapChain->GetSurface()); }


		private:

			LLGL::RenderSystemPtr pRenderer		{ nullptr };
			LLGL::SwapChain* pSwapChain			{ nullptr };
			LLGL::CommandQueue* pCommandQueue	{ nullptr };
			LLGL::CommandBuffer* pCommands		{ nullptr };
			LLGL::PipelineState* pPipeline		{ nullptr };
			LLGL::Buffer* vertexBuffer			{ nullptr };

	};


}