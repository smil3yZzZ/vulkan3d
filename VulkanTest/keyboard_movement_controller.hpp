#pragma once

#include "vk3d_game_object.hpp"
#include "vk3d_window.hpp"

namespace vk3d {
	class KeyboardMovementController {
	public:
		struct KeyMappings {
			int moveLeft = GLFW_KEY_A;
			int moveRight = GLFW_KEY_D;
			int moveForward = GLFW_KEY_W;
			int moveBackward = GLFW_KEY_S;
			int moveUp = GLFW_KEY_SPACE;
			int moveDown = GLFW_KEY_LEFT_SHIFT;
			int lookLeft = GLFW_KEY_LEFT;
			int lookRight = GLFW_KEY_RIGHT;
			int lookUp = GLFW_KEY_UP;
			int lookDown = GLFW_KEY_DOWN;
			int rotateZRight = GLFW_KEY_Z;
			int rotateZLeft = GLFW_KEY_X;
			int cubeFacePositiveX = GLFW_KEY_0;
			int cubeFaceNegativeX = GLFW_KEY_1;
			int cubeFacePositiveY = GLFW_KEY_2;
			int cubeFaceNegativeY = GLFW_KEY_3;
			int cubeFacePositiveZ = GLFW_KEY_4;
			int cubeFaceNegativeZ = GLFW_KEY_5;
		};

		void moveInPlaneXZ(GLFWwindow* window, float dt, Vk3dGameObject& gameObject);

		KeyMappings keys{};
		float moveSpeed{ 3.f };
		float lookSpeed{ 1.5f };
	};
}