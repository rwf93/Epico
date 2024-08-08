#pragma once

class Camera {
public:
    Camera();
    glm::mat4 update(AppContext *context);

    glm::vec3 get_position() { return position; }
    glm::vec3 get_front() { return front; }
    glm::vec3 get_right() { return right; }
    glm::vec3 get_up() { return up; }
private:
    glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 direction;
	glm::vec3 position;
};