#include "camera_entity.h"
#include "move_component.h"
#include "SDL/SDL_scancode.h"
#include "renderer.h"
#include "visualizer.h"

CameraEntity::CameraEntity(Visualizer* vis)
    : Entity(vis)
{
    move_comp_ = new MoveComponent(this);
}

void CameraEntity::UpdateEntity(float deltaTime)
{
    Entity::UpdateEntity(deltaTime);

    Vec3f camera_pos = GetPosition();
    Vec3f target = GetPosition() + GetForward() * 100.0f;
    Vec3f up = Vec3f::UNIT_Z;

    Mat4f view = Mat4f::CreateLookAt(camera_pos, target, up);
    GetVisualizer()->GetRenderer()->SetViewMatrix(view);
}

void CameraEntity::EntityInput(const uint8_t* keys)
{
    float forward_speed = 0.0f;
    float angular_speed = 0.0f;

    if (keys[SDL_SCANCODE_W]) {
        forward_speed += 300.0f;
    }
    if (keys[SDL_SCANCODE_S]) {
        forward_speed -= 300.0f;
    }
    if (keys[SDL_SCANCODE_A]) {
        angular_speed -= Math::TWO_PI<float>;
    }
    if (keys[SDL_SCANCODE_D]) {
        angular_speed += Math::TWO_PI<float>;
    }

    move_comp_->SetForwardSpeed(forward_speed);
    move_comp_->SetAngularSpeed(angular_speed);
}
