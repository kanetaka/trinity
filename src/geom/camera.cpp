#include "geom/camera.h"

using namespace tri;

Camera::Camera(glm::dvec3 position, glm::dvec3 up, float yaw, float pitch)
    : front_(glm::dvec3(0.0, 0.0, -1.0)), movement_speed_(2.5),
    mouse_sensitivity_(0.1), zoom_(45.0)
{
    position_ = position;
    world_up_ = up;
    yaw_ = yaw;
    pitch_ = pitch;
    UpdateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const
{
    return glm::lookAt(glm::vec3(0.0f), glm::vec3(front_), glm::vec3(up_));
}

glm::mat4 Camera::GetProjectionMatrix(float aspect) const
{
    // Note: GLM projection uses OpenGL clip space (-1 to 1 Z).
    // Vulkan clip space is 0 to 1 Z. We will fix this in the projection matrix
    // calculation.
    glm::mat4 proj = glm::perspective(glm::radians(static_cast<float>(zoom_)), aspect, 0.1f, 1000.0f);
    proj[1][1] *= -1; // Invert Y for Vulkan
    return proj;
}

void Camera::ProcessKeyboard(const Uint8* state, float delta_time)
{
    double velocity = movement_speed_ * (double)delta_time;
    if (state[SDL_SCANCODE_W])
        position_ += front_ * velocity;
    if (state[SDL_SCANCODE_S])
        position_ -= front_ * velocity;
    if (state[SDL_SCANCODE_A])
        position_ -= right_ * velocity;
    if (state[SDL_SCANCODE_D])
        position_ += right_ * velocity;
    if (state[SDL_SCANCODE_Q])
        position_ -= up_ * velocity;
    if (state[SDL_SCANCODE_E])
        position_ += up_ * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrain_pitch)
{
    xoffset *= (float)mouse_sensitivity_;
    yoffset *= (float)mouse_sensitivity_;

    yaw_ += xoffset;
    pitch_ += yoffset;

    if (constrain_pitch)
    {
        if (pitch_ > 89.0f)
            pitch_ = 89.0f;
        if (pitch_ < -89.0f)
            pitch_ = -89.0f;
    }

    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset)
{
    zoom_ -= (double)yoffset;
    if (zoom_ < 1.0f)
        zoom_ = 1.0f;
    if (zoom_ > 45.0f)
        zoom_ = 45.0f;
}

void Camera::ProcessMousePanning(float xoffset, float yoffset)
{
    double velocity = mouse_sensitivity_ * 0.1;
    position_ -= right_ * (static_cast<double>(xoffset) * velocity);
    position_ += up_ * (static_cast<double>(yoffset) * velocity);
}

void Camera::UpdateCameraVectors()
{
    glm::dvec3 front;
    front.x = cos(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front.y = sin(glm::radians(pitch_));
    front.z = sin(glm::radians(yaw_)) * cos(glm::radians(pitch_));
    front_ = glm::normalize(front);

    // Normalize the vectors, because their length gets closer to 0 the more you
    // look up or down
    right_ = glm::normalize(glm::cross(front_, world_up_));
    up_ = glm::normalize(glm::cross(right_, front_));
}
