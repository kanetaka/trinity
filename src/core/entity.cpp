#include "core/entity.h"
#include "core/components.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

using namespace tr;


Entity::Entity(Registry& registry, EntityId id)
    : registry_(registry), id_(id)
{
    // Initialize components if they don't exist
    if (!registry_.HasComponent<TransformComponent>(id_))
    {
        registry_.AddComponent<TransformComponent>(id_);
    }
    if (!registry_.HasComponent<HierarchyComponent>(id_))
    {
        registry_.AddComponent<HierarchyComponent>(id_);
    }
}

Entity::~Entity()
{
    registry_.Destroy(id_);
}

void Entity::AddChild(EntityId child_id)
{
    auto* hierarchy = registry_.GetComponent<HierarchyComponent>(id_);
    auto* child_hierarchy = registry_.GetComponent<HierarchyComponent>(child_id);
    
    if (hierarchy && child_hierarchy)
    {
        child_hierarchy->parent = id_;
        hierarchy->children.push_back(child_id);
    }
}

void Entity::RemoveChild(EntityId child_id)
{
    auto* hierarchy = registry_.GetComponent<HierarchyComponent>(id_);
    if (hierarchy)
    {
        auto it = std::find(hierarchy->children.begin(), hierarchy->children.end(), child_id);
        if (it != hierarchy->children.end())
        {
            hierarchy->children.erase(it);
        }
    }
}

void Entity::SetParent(EntityId parent_id)
{
    auto* hierarchy = registry_.GetComponent<HierarchyComponent>(id_);
    if (hierarchy)
    {
        hierarchy->parent = parent_id;
    }
}

EntityId Entity::GetParent()
{
    auto* hierarchy = registry_.GetComponent<HierarchyComponent>(id_);
    if (hierarchy)
    {
        return hierarchy->parent;
    }
    return NullEntity;
}

const std::vector<EntityId>& Entity::GetChildren() const
{
    static const std::vector<EntityId> empty;
    auto* hierarchy = registry_.GetComponent<HierarchyComponent>(id_);
    if (hierarchy)
    {
        return hierarchy->children;
    }
    return empty;
}

const glm::vec3& Entity::GetPosition() const
{
    return registry_.GetComponent<TransformComponent>(id_)->position;
}

void Entity::SetPosition(const glm::vec3& pos)
{
    auto* transform = registry_.GetComponent<TransformComponent>(id_);
    transform->position = pos;
    transform->recompute = true;
}

float Entity::GetScale() const
{
    return registry_.GetComponent<TransformComponent>(id_)->scale;
}

void Entity::SetScale(float scale)
{
    auto* transform = registry_.GetComponent<TransformComponent>(id_);
    transform->scale = scale;
    transform->recompute = true;
}

const glm::quat& Entity::GetRotation() const
{
    return registry_.GetComponent<TransformComponent>(id_)->rotation;
}

void Entity::SetRotation(const glm::quat& rotation)
{
    auto* transform = registry_.GetComponent<TransformComponent>(id_);
    transform->rotation = rotation;
    transform->recompute = true;
}

void Entity::ComputeWorldTransform()
{
    // This is now handled by TransformSystem
}

const glm::mat4& Entity::GetWorldTransform() const
{
    return registry_.GetComponent<TransformComponent>(id_)->world_transform;
}

glm::vec3 Entity::GetForward() const
{
    return GetRotation() * glm::vec3(1.0f, 0.0f, 0.0f);
}

void Entity::Update(float delta_time)
{
    UpdateEntity(delta_time);
}

void Entity::UpdateEntity(float delta_time) {}

void Entity::ProcessInput(const uint8_t* key_state)
{
    EntityInput(key_state);
}

void Entity::EntityInput(const uint8_t* key_state) {}
