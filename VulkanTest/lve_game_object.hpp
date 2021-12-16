#pragma once

#include "lve_model.hpp"

// libs
#include <glm/gtc/matrix_transform.hpp>

// std
#include <memory>

namespace lve {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{1.f, 1.f, 1.f};
		glm::vec3 rotation;

		/*
		// Matrix corresponds to translate * Ry * Rx * Rx * scale transformation
		// Rotation convention uses tait-bryan angles with axis order Y(1), X(2), Z(3)
		glm::mat4 mat4() {
			auto transform = glm::translate(glm::mat4{1.f}, translation);
			transform = glm::rotate(transform, rotation.y, {0.f, 1.f, 0.f});
			transform = glm::rotate(transform, rotation.x, { 1.f, 0.f, 0.f });
			transform = glm::rotate(transform, rotation.z, { 0.f, 0.f, 1.f });
			transform = glm::scale(transform, scale);
			return transform;
		};
		*/
		glm::mat4 mat4() {
			const float c3 = glm::cos(rotation.z);
			const float s3 = glm::sin(rotation.z);
			const float c2 = glm::cos(rotation.x);
			const float s2 = glm::sin(rotation.x);
			const float c1 = glm::cos(rotation.y);
			const float s1 = glm::sin(rotation.y);
			return glm::mat4{
				{
					scale.x * (c1 * c3 + s1 * s2 * s3),
					scale.x * (c2 * s3),
					scale.x * (c1 * s2 * s3 - c3 * s1),
					0.0f,
				},
				{
					scale.y * (c3 * s1 * s2 - c1 * s3),
					scale.y * (c2 * c3),
					scale.y * (c1 * c3 * s2 + s1 * s3),
					0.0f,
				},
				{
					scale.z * (c2 * s1),
					scale.z * (-s2),
					scale.z * (c1 * c2),
					0.0f,
				},
				{translation.x, translation.y, translation.z, 1.0f} };
		}
	};

	class LveGameObject {
	public:
		using id_t = unsigned int;

		static LveGameObject createGameObject() {
			static id_t currentId = 0;
			return LveGameObject{ currentId++ };
		}

		~LveGameObject() {}

		LveGameObject(const LveGameObject &) = delete;
		LveGameObject &operator=(const LveGameObject &) = delete;
		LveGameObject(LveGameObject&&) = default;
		LveGameObject& operator=(LveGameObject&&) = default;

		const id_t getId() { return id; }

		std::shared_ptr<LveModel> model{};
		glm::vec4 color{};
		TransformComponent transform{};

	private:
		LveGameObject(id_t objId) : id{ objId } {}

		id_t id;
	};
}