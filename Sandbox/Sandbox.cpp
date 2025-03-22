#include <Lithe/Lithe.h>

class Sandbox: public Lithe::Application {

	public:

		Sandbox(Lithe::SharedPtr<Lithe::EventDispatcher> dispatcher): Application(dispatcher) {
			std::cout << "Sandbox created\n";
		}

		virtual ~Sandbox() override {
			std::cout << "Sandbox destroyed\n";
		}
	
};

Lithe::Application* create(Lithe::SharedPtr<Lithe::EventDispatcher> dispatcher) {
	return new Sandbox(dispatcher);
}

int main() {
	Lithe::start(&create);
}