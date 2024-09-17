#include "camera.h"

void Camera::update() {
	m.direction.x = cos(glm::radians(m.yaw)) * cos(glm::radians(m.pitch));
	m.direction.y = sin(glm::radians(m.pitch));
	m.direction.z = sin(glm::radians(m.yaw)) * cos(glm::radians(m.pitch));
	m.front = glm::normalize(m.direction);
	m.right = glm::normalize(glm::cross(m.direction, m.up));
}

void Camera::process_event(SDL_Event *event) {
	if(event->type == SDL_MOUSEMOTION) {
		if(!(event->motion.state & SDL_BUTTON(3))) { SDL_SetRelativeMouseMode(SDL_FALSE); return; }
		SDL_SetRelativeMouseMode(SDL_TRUE);
		m.yaw += static_cast<float>(event->motion.xrel) * 0.1f;
		m.pitch -= static_cast<float>(event->motion.yrel) * 0.1f;
	}

	if(event->type == SDL_KEYDOWN) {
		if(event->key.keysym.sym == SDLK_w)
			m.position += m.front * (m.speed);

		if(event->key.keysym.sym == SDLK_s)
			m.position -= m.front * (m.speed);

		if(event->key.keysym.sym == SDLK_d)
			m.position += m.right * (m.speed);

		if(event->key.keysym.sym == SDLK_a)
			m.position -= m.right * (m.speed);
	}
}