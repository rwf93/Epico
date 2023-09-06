#include "mathlib.h"

glm::mat4 mathlib::calculate_model_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
    glm::quat quaternion(rotation);

	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_matrix = glm::toMat4(quaternion);
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);


	return translation_matrix * rotation_matrix * scale_matrix;
}

glm::vec2 mathlib::calculate_billboard(glm::vec3 translation, glm::mat4 projection, glm::mat4 view, int w, int h) {
	glm::vec2 xy;
	
	glm::vec4 world_space = glm::vec4(translation, 1.0f);
	glm::vec4 screen_space = projection * view * world_space;

	screen_space /= screen_space.w;
	xy.x = (screen_space.x + 1.0f) * 0.5f * w;
	xy.y = (screen_space.y + 1.0f) * 0.5f * h;

	return xy;
}