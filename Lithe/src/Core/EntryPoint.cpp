#include "pch.h"

#include "Lithe/Lithe.h"

LITHE_EXPORT void Lithe::start(Lithe::ApplicationCreateFunc create) {

	using namespace Lithe;
	Logger::init("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

	auto dispatcher = makeShared<EventDispatcher>();
	EventBus bus(*dispatcher);


	Application* application = create(dispatcher);
	application->init();


	bus.start();
	application->run();

	Logger::shutdown();

	delete application;

}
