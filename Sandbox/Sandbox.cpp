#include <iostream>

#include <Lithe/Core/EntryPoint.h>

class Sandbox: public Lithe::Application {

	public:

		Sandbox() {
			std::cout << "Sandbox created\n";
		}

		~Sandbox() {
			std::cout << "Sandbox destroyed\n";
		}
	
};

Lithe::Application* Lithe::create() {
	return new Sandbox;
}
