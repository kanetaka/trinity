#pragma once
#include "core/ecs/registry.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace tri
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
        Entity parent = NullEntity;
        std::vector<Entity> children;
    };
} // namespace tri
