#pragma once

#include "entity.h"

class CircleComponent;

class Asteroid : public Entity
{
public:
    Asteroid(class Application* app);
    ~Asteroid();
    CircleComponent* GetCircle() { return circle_; }

private:
    CircleComponent* circle_;
};
