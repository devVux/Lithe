#pragma once

#include "Utils.h"
#include "Window.h"
#include "Scene.h"
#include "Renderer.h"
#include "EventDispatcher.h"

namespace Lithe {

class LITHE_EXPORT Application {

	public:

		Application(SharedPtr<EventDispatcher> dispatcher): pDispatcher(dispatcher) {
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

		SharedPtr<EventDispatcher> pDispatcher;

		SceneManager mSceneManager;

		bool mRunning { false };

	};
	
	typedef Application* (*ApplicationCreateFunc)(SharedPtr<EventDispatcher> dispatcher);

	LITHE_EXPORT void start(ApplicationCreateFunc createFunc);

}
