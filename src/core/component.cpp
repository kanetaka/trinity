#include <cstdint>
#include "core/component.h"
#include "core/entity.h"

namespace trinity::core {




Component::Component(Entity* owner, int update_order)
        : owner_(owner), update_order_(update_order)
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


} // namespace trinity::core
