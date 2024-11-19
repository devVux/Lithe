#pragma once
#include "Renderer.h"

namespace Lithe {

	class Application {

		public:

			Application() {
				std::cout << "Application created\n";
			}
			
			virtual ~Application() {
				std::cout << "Application destroyed\n";
			}

			void init();

			void run();

		private:

			Renderer mRenderer;

	};
	

	extern Application* create();

}