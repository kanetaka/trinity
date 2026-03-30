#pragma once
#include "core/ecs/registry.h"

namespace tri
{
    class TransformSystem
    {
    public:
        static void Update(Registry& registry);
    };
} // namespace tri
