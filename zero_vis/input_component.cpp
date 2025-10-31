#include "input_component.h"
#include "entity.h"

InputComponent::InputComponent(Entity* owner) :
    MoveComponent(owner),
    forward_key_(0),
    back_key_(0),
    clockwise_key_(0),
    counter_clockwise_key_(0)
{
}

void InputComponent::ProcessInput(const uint8_t* key_state)
{
    float forwardSpeed = 0.0f;
    if (key_state[forward_key_]) {
        forwardSpeed += max_forward_speed_;
    }

    if (key_state[back_key_]) {
        forwardSpeed -= max_forward_speed_;
    }
    SetForwardSpeed(forwardSpeed);

    float angular_speed = 0.0f;
    if (key_state[clockwise_key_]) {
        angular_speed += max_angular_speed_;
    }

    if (key_state[counter_clockwise_key_]) {
        angular_speed -= max_angular_speed_;
    }
    SetAngularSpeed(angular_speed);
}
