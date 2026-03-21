#pragma once
#include "core/ecs/registry.h"

namespace trinity::core {




namespace ecs
{
    class TransformSystem
    {
    public:
        static void Update(Registry &registry);
    };
} // namespace ecs


} // namespace trinity::core
