#pragma once

#include <LLGL/LLGL.h>
#include <LLGL/Types.h>
#include <Lithe/Events/EventDispatcher.h>

class GLFWwindow;

namespace Lithe {

	class Window: public LLGL::Surface {
		
		public:

			Window(std::shared_ptr<EventDispatcher> dispatcher, const LLGL::Extent2D& size, const char* title):
				pWindow(CreateGLFWWindow()), mTitle(title), mSize(size), pDispatcher(dispatcher) {
				init(dispatcher);
			}
			virtual ~Window();

			virtual bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)  override;
			virtual bool AdaptForVideoMode(LLGL::Extent2D* resolution, bool* fullscreen) override;
			virtual void ResetPixelFormat() override;

			virtual LLGL::Display* FindResidentDisplay() const override { return LLGL::Display::GetPrimary(); }
			LLGL::Extent2D GetContentSize() const override { return mSize; }


		private:

			void init(std::shared_ptr<EventDispatcher> dispatcher);

			GLFWwindow* CreateGLFWWindow();


		private:

			LLGL::Extent2D mSize;
			std::string mTitle;

			GLFWwindow* pWindow { nullptr };
			std::shared_ptr<EventDispatcher> pDispatcher;


	};

}