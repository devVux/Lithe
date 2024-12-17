#include <Lithe/Lithe.h>

class Sandbox: public Lithe::Application {

	public:

		Sandbox(std::shared_ptr<Lithe::EventDispatcher> dispatcher): Application(dispatcher) {
			std::cout << "Sandbox created\n";
		}

		virtual ~Sandbox() override {
			std::cout << "Sandbox destroyed\n";
		}
	
};

Lithe::Application* Lithe::create(std::shared_ptr<Lithe::EventDispatcher> dispatcher) {
	return new Sandbox(dispatcher);
}
