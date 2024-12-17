#include "pch.h"

#include "Lithe/Lithe.h"

int main() {

	using namespace Lithe;
	Logger::init("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

	auto dispatcher = std::make_shared<EventDispatcher>();
	EventBus bus(*dispatcher);


	Application* application = Lithe::create(dispatcher);
	application->init();



	bus.start();
	application->run();

	Logger::shutdown();

	delete application;

}
