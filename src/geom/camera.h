#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace tri
{
    class Camera
    {
    public:
        Camera(glm::dvec3 position, glm::dvec3 up, float yaw, float pitch);

        glm::mat4 GetViewMatrix() const;
        glm::mat4 GetProjectionMatrix(float aspect) const;

        void ProcessKeyboard(const Uint8* state, float delta_time);
        void ProcessMouseMovement(float xoffset, float yoffset, bool constrain_pitch = true);
        void ProcessMouseScroll(float yoffset);
        void ProcessMousePanning(float xoffset, float yoffset);

        // Accessors
        glm::dvec3 GetPosition() const { return position_; }
        void SetPosition(const glm::dvec3& position) { position_ = position; }

        glm::dvec3 GetFront() const { return front_; }
        glm::dvec3 GetUp() const { return up_; }
        glm::dvec3 GetRight() const { return right_; }

        float GetYaw() const { return yaw_; }
        void SetYaw(float yaw) { yaw_ = yaw; }

        float GetPitch() const { return pitch_; }
        void SetPitch(float pitch) { pitch_ = pitch; }

        double GetZoom() const { return zoom_; }

    private:
        void UpdateCameraVectors();

    private:
        // Camera Attributes
        glm::dvec3 position_;
        glm::dvec3 front_;
        glm::dvec3 up_;
        glm::dvec3 right_;
        glm::dvec3 world_up_;

        // Euler Angles
        float yaw_;
        float pitch_;

        // Camera options
        double movement_speed_;
        double mouse_sensitivity_;
        double zoom_;
    };
} // namespace tri
