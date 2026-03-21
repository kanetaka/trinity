#pragma once
#include "core/ecs/registry.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace trinity::core {
namespace ecs
{
    struct TransformComponent
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::quat rotation = glm::identity<glm::quat>();
        float scale = 1.0f;
        glm::mat4 world_transform = glm::mat4(1.0f);
        bool recompute = true;
    };

    struct HierarchyComponent
    {
        EntityId parent = NullEntity;
        std::vector<EntityId> children;
    };

} // namespace ecs
} // namespace trinity::core

