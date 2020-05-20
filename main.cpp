#include "Samples/RendererApp.h"

int main() {
	RendererApp app;
	app.enableValidation = true;

	try {
		app.Initialize("Hello world", 800, 600);
		app.Run();
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}