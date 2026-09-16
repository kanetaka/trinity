#pragma once
#include <memory>
#include <string>
#include <vector>
#include "scene/object.h"

namespace tri
{
    class Camera;

    // Owns the Object hierarchy and keeps a flat, contiguous index per Object so that
    // per-object GPU data (e.g. transform matrices) can be uploaded as one packed buffer.
    class Scene
    {
    public:
        Scene();

        Object& GetRoot() { return *root_; }
        const Object& GetRoot() const { return *root_; }

        Object& CreateObject(Object& parent, std::string name = "Object");
        void DestroyObject(Object& object);
        void DestroyChildren(Object& object);

        // Advances the scene graph (currently: recomputes world transforms).
        void Update();

        // Performs view-dependent preparation on components implementing IPreRenderable.
        void PreRender(const Camera& camera);

        // Flat view of every live object, indexed by Object::GetTransformIndex().
        const std::vector<Object*>& GetOrderedObjects() const { return ordered_objects_; }

    private:
        void RegisterObject(Object& object);
        void UnregisterSubtree(Object& object);

        std::unique_ptr<Object> root_;
        std::vector<Object*> ordered_objects_;
    };
} // namespace tri
