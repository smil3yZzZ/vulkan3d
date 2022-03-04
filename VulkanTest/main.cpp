#include "vk3d_app.hpp"

// std
#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main() {
	vk3d::Vk3dApp app{};

	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}