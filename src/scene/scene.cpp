#include "scene/scene.h"
#include "scene/component/i_pre_renderable.h"
#include "scene/component/i_renderable.h"
#include <algorithm>

using namespace tri;

Scene::Scene()
    : root_(std::unique_ptr<Object>(new Object("Root")))
{
    root_->scene_ = this;
    RegisterObject(*root_);
}

Object& Scene::CreateObject(Object& parent, std::string name)
{
    auto child = std::unique_ptr<Object>(new Object(std::move(name)));
    Object& ref = *child;
    child->parent_ = &parent;
    child->scene_ = this;
    parent.children_.push_back(std::move(child));
    RegisterObject(ref);
    return ref;
}

void Scene::DestroyObject(Object& object)
{
    Object* parent = object.parent_;
    if (!parent) return; // the root cannot be destroyed

    UnregisterSubtree(object);

    auto& siblings = parent->children_;
    siblings.erase(
        std::remove_if(siblings.begin(), siblings.end(),
            [&](const std::unique_ptr<Object>& child) { return child.get() == &object; }),
        siblings.end());
}

void Scene::DestroyChildren(Object& object)
{
    for (auto& child : object.children_)
    {
        UnregisterSubtree(*child);
    }
    object.children_.clear();
}

void Scene::Update()
{
    root_->UpdateWorldTransform(glm::dmat4(1.0));
}

void Scene::PreRender(const Camera& camera)
{
    for (const auto& entry : pre_render_entries_)
    {
        entry.pre_renderable->PreRender(camera, entry.object->GetWorldTransform());
    }
}

void Scene::RegisterObject(Object& object)
{
    object.scene_ = this;
    object.transform_index_ = static_cast<uint32_t>(ordered_objects_.size());
    ordered_objects_.push_back(&object);

    for (const auto& comp : object.components_)
    {
        RegisterComponent(object, *comp);
    }
}

void Scene::UnregisterSubtree(Object& object)
{
    for (auto& child : object.children_)
    {
        UnregisterSubtree(*child);
    }

    UnregisterComponents(object);

    uint32_t index = object.transform_index_;
    uint32_t last_index = static_cast<uint32_t>(ordered_objects_.size() - 1);
    if (index != last_index)
    {
        ordered_objects_[index] = ordered_objects_[last_index];
        ordered_objects_[index]->transform_index_ = index;
    }
    ordered_objects_.pop_back();
    object.transform_index_ = kInvalidTransformIndex;
    object.scene_ = nullptr;
}

void Scene::SetRenderOrder(IComponent& component, int order)
{
    component.SetRenderOrder(order);
    MarkRenderDirty();
}

void Scene::RegisterComponent(Object& object, IComponent& component)
{
    if (auto* pre_renderable = dynamic_cast<IPreRenderable*>(&component))
    {
        pre_render_entries_.push_back(PreRenderEntry{ pre_renderable, &object });
    }
    if (auto* renderable = dynamic_cast<IRenderable*>(&component))
    {
        render_entries_.push_back(RenderEntry{ renderable, &component, &object });
        MarkRenderDirty();
    }
}

void Scene::UnregisterComponent(Object& object, IComponent& component)
{
    auto* pre_renderable = dynamic_cast<IPreRenderable*>(&component);
    if (pre_renderable)
    {
        pre_render_entries_.erase(
            std::remove_if(pre_render_entries_.begin(), pre_render_entries_.end(),
                [&](const PreRenderEntry& entry)
                {
                    return entry.object == &object && entry.pre_renderable == pre_renderable;
                }),
            pre_render_entries_.end());
    }

    auto* renderable = dynamic_cast<IRenderable*>(&component);
    if (renderable)
    {
        render_entries_.erase(
            std::remove_if(render_entries_.begin(), render_entries_.end(),
                [&](const RenderEntry& entry)
                {
                    return entry.object == &object && entry.renderable == renderable;
                }),
            render_entries_.end());
        MarkRenderDirty();
    }
}

void Scene::UnregisterComponents(Object& object)
{
    pre_render_entries_.erase(
        std::remove_if(pre_render_entries_.begin(), pre_render_entries_.end(),
            [&](const PreRenderEntry& entry)
            {
                return entry.object == &object;
            }),
        pre_render_entries_.end());

    render_entries_.erase(
        std::remove_if(render_entries_.begin(), render_entries_.end(),
            [&](const RenderEntry& entry)
            {
                return entry.object == &object;
            }),
        render_entries_.end());

    MarkRenderDirty();
}
