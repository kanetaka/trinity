#include "core/systems.h"
#include "core/components.h"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

using namespace tri;

static void UpdateTransformRecursive(Registry& registry, Entity entity, const glm::dmat4& parent_world)
{
    auto* transform = registry.GetComponent<TransformComponent>(entity);
    if (!transform) return;

    if (transform->recompute)
    {
        transform->world_transform = glm::translate(glm::dmat4(1.0), transform->position);
        transform->world_transform *= glm::dmat4(glm::mat4_cast(transform->rotation));
        transform->world_transform = glm::scale(transform->world_transform, glm::dvec3(transform->scale));
        transform->recompute = false;
    }

    transform->world_transform = parent_world * transform->world_transform;

    auto* hierarchy = registry.GetComponent<HierarchyComponent>(entity);
    if (hierarchy)
    {
        for (auto child : hierarchy->children)
        {
            UpdateTransformRecursive(registry, child, transform->world_transform);
        }
    }
}

void TransformSystem::Update(Registry& registry)
{
    // Process all roots (entities with Transform but either no Hierarchy or parent is Null)
    registry.ForEach<TransformComponent>([&](Entity entity, TransformComponent& transform)
        {
            auto* hierarchy = registry.GetComponent<HierarchyComponent>(entity);
            if (!hierarchy || hierarchy->parent == NullEntity)
            {
                // It's a root
                if (transform.recompute)
                {
                    transform.world_transform = glm::translate(glm::dmat4(1.0), transform.position);
                    transform.world_transform *= glm::dmat4(glm::mat4_cast(transform.rotation));
                    transform.world_transform = glm::scale(transform.world_transform, glm::dvec3(transform.scale));
                    transform.recompute = false;
                }

                // recurse to children
                if (hierarchy)
                {
                    for (auto child : hierarchy->children)
                    {
                        UpdateTransformRecursive(registry, child, transform.world_transform);
                    }
                }
            }
        });
}
