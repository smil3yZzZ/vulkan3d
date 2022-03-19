#include "vk3d_window.hpp"

#include <stdexcept>

namespace vk3d {
	Vk3dWindow::Vk3dWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	Vk3dWindow::~Vk3dWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void Vk3dWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
	
		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	void Vk3dWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void Vk3dWindow::framebufferResizeCallback(GLFWwindow* window, int width, int height) {
		auto vk3dWindow = reinterpret_cast<Vk3dWindow*>(glfwGetWindowUserPointer(window));
		vk3dWindow->framebufferResized = true;
		vk3dWindow->width = width;
		vk3dWindow->height = height;
	}

}