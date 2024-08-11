#pragma once

class Camera {
public:
    Camera(AppContext *appcontext):
        context(appcontext), right(glm::normalize(glm::cross(front, up))) {}

    void process_event(SDL_Event *event);
    void update();

    glm::vec3 get_position() { return position; }
    glm::vec3 get_front() { return front; }
    glm::vec3 get_right() { return right; }
    glm::vec3 get_up() { return up; }
    glm::mat4 get_view_matrix() { return glm::lookAt(get_position(), get_position() + get_front(), get_up()); };

private:
    AppContext *context;

    glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 right;
	glm::vec3 direction = glm::vec3(0);
	glm::vec3 position = glm::vec3(0);

    float yaw = 0.0f;
    float pitch = 0.0f;
    float speed = 0.1f;
};