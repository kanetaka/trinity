#include "component.h"
#include "entity.h"

Component::Component(Entity* owner, int update_order)
    : owner_(owner),
      update_order_(update_order)
{
    owner_->AddComponent(this);
}

Component::~Component()
{
    owner_->RemoveComponent(this);
}

void Component::Update(float delta_time)
{
}
