#pragma once

namespace mathlib {

glm::mat4 calculate_model_matrix(glm::vec3 translation, glm::vec3 rotation, glm::vec3 scale);
glm::vec2 calculate_billboard(glm::vec3 translation, glm::mat4 projection, glm::mat4 view, int w, int h);

}