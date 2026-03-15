#pragma once
#include "core/ecs/registry.h"

namespace ecs {

class TransformSystem {
public:
    static void Update(Registry& registry);
};

} // namespace ecs
