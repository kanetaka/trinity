#pragma once
#include <glm/glm.hpp>

namespace tri
{
    class Camera;

    // Interface for components that require preparation (e.g. sorting, culling) before rendering.
    class IPreRenderable
    {
    public:
        virtual ~IPreRenderable() = default;

        virtual void PreRender(const Camera& camera, const glm::dmat4& world_transform) = 0;
    };
} // namespace tri
