#pragma once
#include <vector>
#include <cstdint>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "core/registry.h"

namespace tr {
class Entity
{
public:
    Entity(Registry& registry, EntityId id);
    virtual ~Entity();

    EntityId GetId() const { return id_; }

    void AddChild(EntityId child_id);
    void RemoveChild(EntityId child_id);
    void SetParent(EntityId parent_id);
    EntityId GetParent();
    const std::vector<EntityId>& GetChildren() const;

    void Update(float delta_time);
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

    Registry& GetRegistry() { return registry_; }

private:
    EntityId id_;
    Registry& registry_;
};
} // namespace tr
