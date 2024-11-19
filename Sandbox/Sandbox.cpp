#include <Lithe/Lithe.h>

class Sandbox: public Lithe::Application {

	public:

		Sandbox() {
			std::cout << "Sandbox created\n";
		}

		virtual ~Sandbox() override {
			std::cout << "Sandbox destroyed\n";
		}
	
};

Lithe::Application* Lithe::create() {
	return new Sandbox;
}
