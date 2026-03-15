#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include "entity.h"
#include "component.h"
#include "application.h"
#include "core/renderer.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <cstdint>

// Forward declarations
class Application;
class Component;

Entity::Entity(Application* app)
        : state_(EActive), position_(0.0f, 0.0f, 0.0f), rotation_(glm::identity<glm::quat>()),
            scale_(1.0f), recompute_world_transform_(true), app_(app)
{
    // game_->AddEntity(this); // This should be called by GsApp or similar
}

Entity::~Entity()
{
    // game_->RemoveEntity(this);
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
    for (auto comp : components_)
    {
        comp->Update(delta_time);
    }
}

void Entity::UpdateEntity(float delta_time) {}

void Entity::ProcessInput(const uint8_t* key_state)
{
    if (state_ == EActive) {
        for (auto comp : components_)
        {
            comp->ProcessInput(key_state);
        }
        EntityInput(key_state);
    }
}

void Entity::EntityInput(const uint8_t* key_state) {}

void Entity::ComputeWorldTransform()
{
    if (recompute_world_transform_)
    {
        recompute_world_transform_ = false;
        // Scale, then rotate, then translate
        world_transform_ = glm::translate(glm::mat4(1.0f), position_);
        world_transform_ *= glm::mat4_cast(rotation_);
        world_transform_ = glm::scale(world_transform_, glm::vec3(scale_));

        // Inform components world transform updated
        for (auto comp : components_)
        {
            comp->OnUpdateWorldTransform();
        }
    }
}

void Entity::AddComponent(Component* component)
{
    // Find the insertion point before the first element with a higher update order
    int my_order = component->GetUpdateOrder();
    auto iter = components_.begin();
    for (; iter != components_.end(); ++iter)
    {
        if (my_order < (*iter)->GetUpdateOrder())
        {
            break;
        }
    }
    components_.insert(iter, component);
}

void Entity::RemoveComponent(Component* component)
{
    auto iter = std::find(components_.begin(), components_.end(), component);
    if (iter != components_.end())
    {
        components_.erase(iter);
    }
}
