#include "camera.h"

void Camera::update() {
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(direction);
	right = glm::normalize(glm::cross(direction, up));
}

void Camera::process_event(SDL_Event *event) {
	if(event->type == SDL_MOUSEMOTION) {
		if(!(event->motion.state & SDL_BUTTON(3))) { SDL_SetRelativeMouseMode(SDL_FALSE); return; }
		SDL_WarpMouseInWindow(context->window, context->width / 2, context->height / 2);
		SDL_SetRelativeMouseMode(SDL_TRUE);
		yaw += static_cast<float>(event->motion.xrel) * 0.1f;
		pitch -= static_cast<float>(event->motion.yrel) * 0.1f;
	}

	if(event->type == SDL_KEYDOWN) {
		if(event->key.keysym.sym == SDLK_w)
			position += front * (speed);

		if(event->key.keysym.sym == SDLK_s)
			position -= front * (speed);

		if(event->key.keysym.sym == SDLK_d)
			position += right * (speed);

		if(event->key.keysym.sym == SDLK_a)
			position -= right * (speed);
	}
}