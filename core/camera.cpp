#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f),
      MouseSensitivity(0.1f), Zoom(45.0f) {
  Position = position;
  WorldUp = up;
  Yaw = yaw;
  Pitch = pitch;
  updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
  return glm::lookAt(Position, Position + Front, Up);
}

glm::mat4 Camera::GetProjectionMatrix(float aspect) const {
  // Note: GLM projection uses OpenGL clip space (-1 to 1 Z).
  // Vulkan clip space is 0 to 1 Z. We will fix this in the projection matrix
  // calculation.
  glm::mat4 proj = glm::perspective(glm::radians(Zoom), aspect, 0.1f, 1000.0f);
  proj[1][1] *= -1; // Invert Y for Vulkan
  return proj;
}

void Camera::ProcessKeyboard(const Uint8 *state, float deltaTime) {
  float velocity = MovementSpeed * deltaTime;
  if (state[SDL_SCANCODE_W])
    Position += Front * velocity;
  if (state[SDL_SCANCODE_S])
    Position -= Front * velocity;
  if (state[SDL_SCANCODE_A])
    Position -= Right * velocity;
  if (state[SDL_SCANCODE_D])
    Position += Right * velocity;
  if (state[SDL_SCANCODE_Q])
    Position -= Up * velocity;
  if (state[SDL_SCANCODE_E])
    Position += Up * velocity;
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset,
                                  bool constrainPitch) {
  xoffset *= MouseSensitivity;
  yoffset *= MouseSensitivity;

  Yaw += xoffset;
  Pitch += yoffset;

  if (constrainPitch) {
    if (Pitch > 89.0f)
      Pitch = 89.0f;
    if (Pitch < -89.0f)
      Pitch = -89.0f;
  }

  updateCameraVectors();
}

void Camera::updateCameraVectors() {
  glm::vec3 front;
  front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  front.y = sin(glm::radians(Pitch));
  front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
  Front = glm::normalize(front);

  // Normalize the vectors, because their length gets closer to 0 the more you
  // look up or down
  Right = glm::normalize(glm::cross(Front, WorldUp));
  Up = glm::normalize(glm::cross(Right, Front));
}
