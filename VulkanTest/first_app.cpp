#include "first_app.hpp"
#include "simple_render_system.hpp"

#include <math.h>

// libs
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>

#include <iostream>


namespace lve {

	FirstApp::FirstApp() {
		loadGameObjects();
	}

	FirstApp::~FirstApp() {
	}

	void FirstApp::run() {
		SimpleRenderSystem simpleRenderSystem{lveDevice, lveRenderer.getSwapChainRenderPass()};
		//int frame = 0;
		//int powIteration = 1;
		while (!lveWindow.shouldClose()) {
			/*
			if (frame > 0 && frame % 60 == 0 && frame < 60 * SIERPINSKI_DEPTH) {
				updateModels(powIteration++);
			}
			*/
			glfwPollEvents();
			if (auto commandBuffer = lveRenderer.beginFrame()) {
				lveRenderer.beginSwapChainRenderPass(commandBuffer);
				//Set this to allow multiple vertex buffers!
				//renderGameObjects(lveRenderer.getCurrentFrame(), commandBuffer);
				simpleRenderSystem.renderGameObjects(lveRenderer.getFrameIndex(), commandBuffer, gameObjects, vertices);
				lveRenderer.endSwapChainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
			//frame++;
		}
		vkDeviceWaitIdle(lveDevice.device());
	}

	void FirstApp::loadGameObjects() {
		vertices = {
			{{0.0f, -0.5f}, {1.0f, 1.0f, 0.0f, 0.5f}},
			{{0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 0.5f}},
			{{-0.5f, 0.5f}, {1.0f, 1.0f, 0.0f, 0.5f}}
		};
		//Set this to allow multiple vertex buffers!
		//auto lveModel = std::make_shared<LveModel>(0, lveDevice, vertices, lveAllocator);
		auto lveModel = std::make_shared<LveModel>(lveDevice, vertices, lveAllocator);

		auto triangle = LveGameObject::createGameObject();
		triangle.model = lveModel;
		triangle.color = {.1f, .8f, .1f, 1.0f};
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = { 2.f, .5f };
		triangle.transform2d.rotation = .25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}

	void FirstApp::updateModels(int powIteration) {
		int numOfVertices = NUMBER_OF_TRIANGLE_VERTICES;
		int previousNumOfTriangles = pow(numOfVertices, (double)powIteration - 1);
		int numOfTrianglesPerPreviousTriangles = pow(numOfVertices, (double)powIteration)/previousNumOfTriangles;
		std::vector<LveModel::Vertex> auxVertices(numOfVertices * previousNumOfTriangles * numOfTrianglesPerPreviousTriangles);
		for (int i = 0; i < previousNumOfTriangles; i++) {
			for (int j = 0; j < numOfTrianglesPerPreviousTriangles; j++) {
				for (int k = 0; k < numOfVertices; k++) {
					auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].position = (vertices[i * numOfVertices + j].position + vertices[i * numOfVertices + k].position) / 2.0f;
					auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].color = vertices[i * numOfVertices + j].color;
					auxVertices[i * numOfTrianglesPerPreviousTriangles * numOfVertices + j * numOfVertices + k].color[3] = 0.5f;
				}
			}
		}
		vertices = auxVertices;
	}

}