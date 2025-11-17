#include "entity.h"
#include "visualizer.h"
#include "component.h"
#include <algorithm>

Entity::Entity(Visualizer* vis) :
    state_(EActive),
    position_(Vec3f::ZERO),
    rotation_(Quatf::IDENTITY),
    scale_(1.0f),
    vis_(vis),
    recompute_world_transform_(true)
{
    vis_->AddEntity(this);
}

Entity::~Entity()
{
    vis_->RemoveEntity(this);

    while (!components_.empty()) {
        delete components_.back();
    }
}

void Entity::Update(float delta_time)
{
    if (state_ == EActive) {
        ComputeWorldTransform();
        UpdateComponents(delta_time);
        UpdateEntity(delta_time);
        ComputeWorldTransform();
    }
}

void Entity::UpdateComponents(float delta_time)
{
    for (auto comp : components_) {
        comp->Update(delta_time);
    }
}

void Entity::UpdateEntity(float delta_time)
{
}

void Entity::ProcessInput(const uint8_t* key_state)
{
    if (state_ == EActive) {
        for (auto comp : components_) {
            comp->ProcessInput(key_state);
        }
        EntityInput(key_state);
    }
}

void Entity::EntityInput(const uint8_t* key_state)
{
}

void Entity::ComputeWorldTransform()
{
    if (recompute_world_transform_) {
        recompute_world_transform_ = false;

        world_transform_  = Mat4f::CreateTranslation(position_);
        world_transform_ *= Mat4f::FromQuaternion(rotation_);
        world_transform_ *= Mat4f::CreateScale(scale_);

        for (auto comp : components_) {
            comp->OnUpdateWorldTransform();
        }
    }
}

void Entity::AddComponent(Component* component)
{
    int my_order = component->GetUpdateOrder();
    auto iter = components_.begin();
    for (; iter != components_.end(); ++iter) {
        if (my_order < (*iter)->GetUpdateOrder()) {
            break;
        }
    }

    components_.insert(iter, component);
}

void Entity::RemoveComponent(Component* component)
{
    auto iter = std::find(components_.begin(), components_.end(), component);
    if (iter != components_.end()) {
        components_.erase(iter);
    }
}
