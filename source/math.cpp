#include "math.h"

glm::mat4 math::calculate_model_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale) {
    glm::quat quaternion(rotation);

	glm::mat4 translation_matrix = glm::translate(glm::mat4(1.0f), translation);
	glm::mat4 rotation_matrix = glm::toMat4(quaternion);
	glm::mat4 scale_matrix = glm::scale(glm::mat4(1.0f), scale);


	return translation_matrix * rotation_matrix * scale_matrix;
}