#pragma once

#include "entity.h"

class Laser : public Entity
{
public:
    Laser(class Application* app);
    void UpdateEntity(float deltaTime) override;
private:
    class CircleComponent* circle_;
    float death_timer_;
};
