#include "camera.h"

Camera::Camera() {
	front = glm::vec3(0.0f, 0.0f, -1.0f);
    up    = glm::vec3(0.0f, 1.0f, 0.0f);;
    right = glm::normalize(glm::cross(front, up));
}

glm::mat4 Camera::update(AppContext *context) {
	int mx = 0, my = 0;

	SDL_PumpEvents();
	const Uint32 mouse_state = SDL_GetMouseState(&mx, &my);
	const Uint8* key_state = SDL_GetKeyboardState(NULL);

	static float pitch = 0.0f;
	static float yaw = -90.0f;
	const float sensitivity = 0.1f;

	static float last_mx = 400.0f, last_my = 300.0f;

	float offset_mx = (float)mx - last_mx;
	float offset_my = last_my - (float)my;

	last_mx = static_cast<float>(mx);
	last_my = static_cast<float>(my);

	if(mouse_state & SDL_BUTTON(3)) {
		offset_mx *= sensitivity;
		offset_my *= sensitivity;

		yaw += offset_mx;
		pitch += offset_my;
	}

	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));

	front = glm::normalize(direction);
	right = glm::normalize(glm::cross(direction, up));

	float camera_speed = 8.0f;

	if(key_state[SDL_SCANCODE_LSHIFT])
		camera_speed *= 6.0f;

	if(key_state[SDL_SCANCODE_W])
		position += front * (context->time_delta * camera_speed);

	if(key_state[SDL_SCANCODE_S])
		position -= front * (context->time_delta * camera_speed);

	if(key_state[SDL_SCANCODE_D])
		position += right * (context->time_delta * camera_speed);

	if(key_state[SDL_SCANCODE_A])
		position -= right * (context->time_delta * camera_speed);

    return glm::lookAt(get_position(), get_position() + get_front(), get_up());
}