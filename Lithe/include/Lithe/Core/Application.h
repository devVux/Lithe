#pragma once

#include "Renderer.h"
#include "Window.h"
#include "EventDispatcher.h"
#include "Lithe/Scene/Scene.h"
#include "Export.h"

namespace Lithe {

class LITHE_EXPORT Application {

		public:

			Application(EventDispatcher& dispatcher): mDispatcher(dispatcher) {
				std::cout << "Application created\n";
			}
			
			virtual ~Application() {
				stop();
				std::cout << "Application destroyed\n";
			}

			void init();

			void start() { mRunning = true; }
			void stop() { mRunning = false; }
			void run();


		private:

			Renderer mRenderer; 
			SharedPtr<Window> pWindow { nullptr };

			EventDispatcher& mDispatcher;

			SceneManager mSceneManager;

			bool mRunning { false };

	};
	
	typedef Application* (*ApplicationCreateFunc)(EventDispatcher& dispatcher);

	LITHE_EXPORT void start(ApplicationCreateFunc createFunc);

}
