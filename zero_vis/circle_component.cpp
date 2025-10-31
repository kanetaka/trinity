#include "circle_component.h"
#include "entity.h"

CircleComponent::CircleComponent(Entity* owner) :
    Component(owner),
    radius_(0.0f)
{
}

const Vec3f& CircleComponent::GetCenter() const
{
    return owner_->GetPosition();
}

float CircleComponent::GetRadius() const
{
    return owner_->GetScale() * radius_;
}

bool Intersect(const CircleComponent& a, const CircleComponent& b)
{
    Vec3f diff = a.GetCenter() - b.GetCenter();
    float dist_sq = diff.LengthSq();

    float radii_sq = a.GetRadius() + b.GetRadius();
    radii_sq *= radii_sq;

    return dist_sq <= radii_sq;
}
