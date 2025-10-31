#pragma once

#include "component.h"

class Entity;

class MoveComponent : public Component
{
public:
    MoveComponent(Entity* owner, int updateOrder = 10);

public:
    void Update(float delta_time) override;

    float GetAngularSpeed() const { return angular_speed_; }
    float GetForwardSpeed() const { return forward_speed_; }
    void SetAngularSpeed(float speed) { angular_speed_ = speed; }
    void SetForwardSpeed(float speed) { forward_speed_ = speed; }

private:
    float angular_speed_;
    float forward_speed_;
};
