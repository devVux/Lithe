#pragma once

#include "Renderer.h"
#include "Lithe/Core/Window.h"
#include "Lithe/Events/EventDispatcher.h"

namespace Lithe {

	class Application {

		public:

			Application(std::shared_ptr<EventDispatcher> dispatcher): pDispatcher(dispatcher) {
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
			std::shared_ptr<Window> pWindow { nullptr };

			std::shared_ptr<EventDispatcher> pDispatcher;

			bool mRunning { false };

	};
	

	extern Application* create(std::shared_ptr<EventDispatcher>);

}