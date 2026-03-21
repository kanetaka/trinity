#pragma once
#include "core/registry.h"

namespace trinity::core {
class TransformSystem
{
public:
    static void Update(Registry &registry);
};
} // namespace trinity::core
