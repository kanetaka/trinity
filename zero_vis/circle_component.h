#pragma once

#include "component.h"
#include "vector.h"

class Entity;

class CircleComponent : public Component
{
public:
    CircleComponent(Entity* owner);

public:
    void SetRadius(float radius) { radius_ = radius; }
    float GetRadius() const;
    const Vec3f& GetCenter() const;

private:
    float radius_;
};

bool Intersect(const CircleComponent& a, const CircleComponent& b);
