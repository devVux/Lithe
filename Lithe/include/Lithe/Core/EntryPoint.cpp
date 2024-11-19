#include "pch.h"

#include "Lithe/Lithe.h"

int main() {

	using namespace Lithe;

	Logger::init("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

	Application* application = Lithe::create();
	application->init();

	application->run();

	Logger::shutdown();

	delete application;

}
