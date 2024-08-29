#pragma once
#include <iostream>

namespace Lithe {

	class Application {

		public:

			Application() {
				std::cout << "Application created\n";
			}
			
			virtual ~Application() {
				std::cout << "Application destroyed\n";
			}


	};
	

	extern Application* create();

}