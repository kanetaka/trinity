#pragma once

#include "move_component.h"
#include <cstdint>

class Entity;

class InputComponent : public MoveComponent
{
public:
    InputComponent(Entity* owner);

public:
    void ProcessInput(const uint8_t* key_state) override;

    float GetMaxForward() const { return max_forward_speed_; }
    float GetMaxAngular() const { return max_angular_speed_; }
    int GetForwardKey() const { return forward_key_; }
    int GetBackKey() const { return back_key_; }
    int GetClockwiseKey() const { return clockwise_key_; }
    int GetCounterClockwiseKey() const { return counter_clockwise_key_; }

    void SetMaxForwardSpeed(float speed) { max_forward_speed_ = speed; }
    void SetMaxAngularSpeed(float speed) { max_angular_speed_ = speed; }
    void SetForwardKey(int key) { forward_key_ = key; }
    void SetBackKey(int key) { back_key_ = key; }
    void SetClockwiseKey(int key) { clockwise_key_ = key; }
    void SetCounterClockwiseKey(int key) { counter_clockwise_key_ = key; }

private:
    float max_forward_speed_;
    float max_angular_speed_;
    int forward_key_;
    int back_key_;
    int clockwise_key_;
    int counter_clockwise_key_;
};
