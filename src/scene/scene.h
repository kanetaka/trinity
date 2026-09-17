#pragma once
#include <memory>
#include <string>
#include <vector>
#include "scene/object.h"

namespace tri
{
    class Camera;
    class IPreRenderable;
    class IRenderable;
    class IComponent;

    struct PreRenderEntry
    {
        IPreRenderable* pre_renderable;
        const Object* object;
    };

    struct RenderEntry
    {
        IRenderable* renderable;
        const IComponent* component;
        const Object* object;
    };

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

        // Cached entries for pre-rendering and rendering
        const std::vector<PreRenderEntry>& GetPreRenderEntries() const { return pre_render_entries_; }
        const std::vector<RenderEntry>& GetRenderEntries() const { return render_entries_; }

        bool IsRenderDirty() const { return is_render_dirty_; }
        void MarkRenderDirty() { is_render_dirty_ = true; }
        void ClearRenderDirty() const { is_render_dirty_ = false; }
        void SetRenderOrder(IComponent& component, int order);

        void RegisterComponent(Object& object, IComponent& component);
        void UnregisterComponent(Object& object, IComponent& component);

    private:
        void RegisterObject(Object& object);
        void UnregisterSubtree(Object& object);
        void UnregisterComponents(Object& object);

        std::unique_ptr<Object> root_;
        std::vector<Object*> ordered_objects_;
        std::vector<PreRenderEntry> pre_render_entries_;
        std::vector<RenderEntry> render_entries_;
        mutable bool is_render_dirty_ = true;
    };
} // namespace tri
