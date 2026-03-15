#include "entity.h"
#include "component.h"
#include "core/ecs/components.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

Entity::Entity(ecs::Registry& registry, ecs::EntityId id)
    : registry_(registry), id_(id)
{
    // Initialize components if they don't exist
    if (!registry_.HasComponent<ecs::TransformComponent>(id_)) {
        registry_.AddComponent<ecs::TransformComponent>(id_);
    }
    if (!registry_.HasComponent<ecs::HierarchyComponent>(id_)) {
        registry_.AddComponent<ecs::HierarchyComponent>(id_);
    }
}

Entity::~Entity()
{
    for (auto comp : components_) {
        delete comp;
    }
    registry_.Destroy(id_);
}

void Entity::AddChild(ecs::EntityId child_id)
{
    auto* hierarchy = registry_.GetComponent<ecs::HierarchyComponent>(id_);
    auto* child_hierarchy = registry_.GetComponent<ecs::HierarchyComponent>(child_id);
    
    if (hierarchy && child_hierarchy) {
        child_hierarchy->parent = id_;
        hierarchy->children.push_back(child_id);
    }
}

void Entity::RemoveChild(ecs::EntityId child_id)
{
    auto* hierarchy = registry_.GetComponent<ecs::HierarchyComponent>(id_);
    if (hierarchy) {
        auto it = std::find(hierarchy->children.begin(), hierarchy->children.end(), child_id);
        if (it != hierarchy->children.end()) {
            hierarchy->children.erase(it);
        }
    }
}

void Entity::SetParent(ecs::EntityId parent_id)
{
    auto* hierarchy = registry_.GetComponent<ecs::HierarchyComponent>(id_);
    if (hierarchy) {
        hierarchy->parent = parent_id;
    }
}

ecs::EntityId Entity::GetParent()
{
    auto* hierarchy = registry_.GetComponent<ecs::HierarchyComponent>(id_);
    if (hierarchy) {
        return hierarchy->parent;
    }
    return ecs::NullEntity;
}

const std::vector<ecs::EntityId>& Entity::GetChildren() const
{
    static const std::vector<ecs::EntityId> empty;
    auto* hierarchy = registry_.GetComponent<ecs::HierarchyComponent>(id_);
    if (hierarchy) {
        return hierarchy->children;
    }
    return empty;
}

const glm::vec3& Entity::GetPosition() const {
    return registry_.GetComponent<ecs::TransformComponent>(id_)->position;
}

void Entity::SetPosition(const glm::vec3& pos) {
    auto* transform = registry_.GetComponent<ecs::TransformComponent>(id_);
    transform->position = pos;
    transform->recompute = true;
}

float Entity::GetScale() const {
    return registry_.GetComponent<ecs::TransformComponent>(id_)->scale;
}

void Entity::SetScale(float scale) {
    auto* transform = registry_.GetComponent<ecs::TransformComponent>(id_);
    transform->scale = scale;
    transform->recompute = true;
}

const glm::quat& Entity::GetRotation() const {
    return registry_.GetComponent<ecs::TransformComponent>(id_)->rotation;
}

void Entity::SetRotation(const glm::quat& rotation) {
    auto* transform = registry_.GetComponent<ecs::TransformComponent>(id_);
    transform->rotation = rotation;
    transform->recompute = true;
}

void Entity::ComputeWorldTransform() {
    // This is now handled by TransformSystem
}

const glm::mat4& Entity::GetWorldTransform() const {
    return registry_.GetComponent<ecs::TransformComponent>(id_)->world_transform;
}

glm::vec3 Entity::GetForward() const {
    return GetRotation() * glm::vec3(1.0f, 0.0f, 0.0f);
}

void Entity::AddComponent(Component* component)
{
    components_.push_back(component);
}

void Entity::RemoveComponent(Component* component)
{
    auto it = std::find(components_.begin(), components_.end(), component);
    if (it != components_.end()) {
        components_.erase(it);
    }
}

void Entity::Update(float delta_time)
{
    UpdateComponents(delta_time);
    UpdateEntity(delta_time);
    
    // Child update should be handled by the application or a system, 
    // but for now keeping it recursive via ID lookup if needed.
    // However, simplest is to let Application::UpdateEntities handle it if it iterates all.
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
    for (auto comp : components_)
    {
        comp->ProcessInput(key_state);
    }
    EntityInput(key_state);
}

void Entity::EntityInput(const uint8_t* key_state) {}
