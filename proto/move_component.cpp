#include "move_component.h"
#include "entity.h"

MoveComponent::MoveComponent(Entity* owner, int update_order) :
    Component(owner, update_order),
    angular_speed_(0.0f),
    forward_speed_(0.0f)
{
}

void MoveComponent::Update(float delta_time)
{
    if (!Math::IsNearZero(angular_speed_)) {
        Quatf rot = owner_->GetRotation();
        float angle = angular_speed_ * delta_time;
        // Create quaternion for incremental rotation
        // (Rotate about up axis)
        Quatf inc(Vec3f::UNIT_Z, angle); // TODO check
        rot = Quatf::Concatenate(rot, inc);
        owner_->SetRotation(rot);
    }

    if (!Math::IsNearZero(forward_speed_)) {
        Vec3f pos = owner_->GetPosition();
        pos += owner_->GetForward() * forward_speed_ * delta_time;
        owner_->SetPosition(pos);
    }
}
