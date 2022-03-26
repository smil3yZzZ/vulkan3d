#pragma once

#include "vk3d_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>
#include <unordered_map>

namespace vk3d {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation{};

		glm::mat4 mat4();
		glm::mat3 normalMatrix();
		void resetRotation();
	};

	class Vk3dGameObject {
	public:
		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, Vk3dGameObject>;

		static Vk3dGameObject createGameObject() {
			static id_t currentId = 0;
			return Vk3dGameObject{ currentId++ };
		}

		~Vk3dGameObject() {}

		Vk3dGameObject(const Vk3dGameObject &) = delete;
		Vk3dGameObject &operator=(const Vk3dGameObject &) = delete;
		Vk3dGameObject(Vk3dGameObject&&) = default;
		Vk3dGameObject& operator=(Vk3dGameObject&&) = default;

		const id_t getId() { return id; }

		std::shared_ptr<Vk3dModel> model{};
		glm::vec4 color{};
		TransformComponent transform{};
		float reflection = 0.0f;

	private:
		Vk3dGameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}