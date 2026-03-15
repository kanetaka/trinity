#pragma once
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>

#include "core/ecs/registry.h"

class Application;
class Component;

class Entity
{
public:
    Entity(ecs::Registry& registry, ecs::EntityId id);
    virtual ~Entity();

    ecs::EntityId GetId() const { return id_; }

    void AddChild(ecs::EntityId child_id);
    void RemoveChild(ecs::EntityId child_id);
    void SetParent(ecs::EntityId parent_id);
    ecs::EntityId GetParent();
    const std::vector<ecs::EntityId>& GetChildren() const;

    void Update(float delta_time);
    void UpdateComponents(float delta_time);
    virtual void UpdateEntity(float delta_time);

    void ProcessInput(const uint8_t* key_state);
    virtual void EntityInput(const uint8_t* key_state);

    const glm::vec3& GetPosition() const;
    void SetPosition(const glm::vec3& pos);
    float GetScale() const;
    void SetScale(float scale);
    const glm::quat& GetRotation() const;
    void SetRotation(const glm::quat& rotation);

    void ComputeWorldTransform();
    const glm::mat4& GetWorldTransform() const;

    glm::vec3 GetForward() const;

    ecs::Registry& GetRegistry() { return registry_; }

    void AddComponent(Component* component);
    void RemoveComponent(Component* component);
    const std::vector<Component*>& GetComponents() const { return components_; }

private:
    ecs::EntityId id_;
    std::vector<Component*> components_;
    ecs::Registry& registry_;
};
