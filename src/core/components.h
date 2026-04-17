#pragma once
#include "core/registry.h"
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>

namespace tri
{
    struct TransformComponent
    {
        glm::dvec3 position = glm::dvec3(0.0);
        glm::quat rotation = glm::identity<glm::quat>();
        float scale = 1.0f;
        glm::dmat4 world_transform = glm::dmat4(1.0);
        bool recompute = true;
    };

    struct HierarchyComponent
    {
        Entity parent = NullEntity;
        std::vector<Entity> children;
    };
} // namespace tri
