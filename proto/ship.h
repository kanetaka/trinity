#pragma once

#include "entity.h"

class Application;

class Ship : public Entity
{
public:
    Ship(Application* app);

    void UpdateEntity(float delta_time) override;
    void EntityInput(const uint8_t* key_state) override;
private:
    float laser_cool_down_;
};
