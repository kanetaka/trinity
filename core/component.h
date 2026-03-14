#pragma once
#include <cstdint>

class Actor;

class Component {
public:
    Component(Actor* owner, int update_order = 100);
    virtual ~Component();

    virtual void Update(float delta_time);
    virtual void ProcessInput(const uint8_t* key_state) {}
    virtual void OnUpdateWorldTransform() {}

    int GetUpdateOrder() const { return update_order_; }

protected:
    Actor* owner_;
    int update_order_;
};
