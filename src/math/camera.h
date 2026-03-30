#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace tri
{
    class Camera
    {
    public:
        Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix(float aspect) const;

        void ProcessKeyboard(const Uint8* state, float deltaTime);
        void ProcessMouseMovement(float xoffset, float yoffset,
            bool constrainPitch = true);
        void ProcessMouseScroll(float yoffset);
        void ProcessMousePanning(float xoffset, float yoffset);

        // Camera Attributes
        glm::vec3 Position;
        glm::vec3 Front;
        glm::vec3 Up;
        glm::vec3 Right;
        glm::vec3 WorldUp;

        // Euler Angles
        float Yaw;
        float Pitch;

        // Camera options
        float MovementSpeed;
        float MouseSensitivity;
        float Zoom;

    private:
        void UpdateCameraVectors();
    };
} // namespace tri
