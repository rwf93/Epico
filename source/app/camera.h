#pragma once

class Camera {
	struct M {
		AppContext *context;

		glm::vec3 front     = glm::vec3(0);
		glm::vec3 up        = glm::vec3(0);
		glm::vec3 right     = glm::vec3(0);
		glm::vec3 direction = glm::vec3(0);
		glm::vec3 position  = glm::vec3(0);

		float yaw = 0.0f;
		float pitch = 0.0f;
		float speed = 0.0f;
	} m;

	explicit Camera(M m) : m(std::move(m)) {}
public:
	static Camera create(AppContext *context, float speed = 0.1) {
		auto front = glm::vec3(0.0f, 0.0f, -1.0f);
		auto up = glm::vec3(0.0f, 1.0f, 0.0f);
		auto right = glm::normalize(glm::cross(front, up));

		return Camera(M{
			.context = context,
			.front = front,
			.up = up,
			.right = right,
			.speed = speed
		});
	}

	void process_event(SDL_Event *event);
	void update();

	glm::vec3 get_position() { return m.position; }
	glm::vec3 get_front() { return m.front; }
	glm::vec3 get_right() { return m.right; }
	glm::vec3 get_up() { return m.up; }
	glm::mat4 get_view_matrix() { return glm::lookAt(get_position(), get_position() + get_front(), get_up()); };
};