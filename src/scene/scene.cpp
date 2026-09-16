#include "scene/scene.h"
#include "scene/component/i_pre_renderable.h"
#include <algorithm>

using namespace tri;

Scene::Scene()
    : root_(std::unique_ptr<Object>(new Object("Root")))
{
    RegisterObject(*root_);
}

Object& Scene::CreateObject(Object& parent, std::string name)
{
    auto child = std::unique_ptr<Object>(new Object(std::move(name)));
    Object& ref = *child;
    child->parent_ = &parent;
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
    for (auto* object : ordered_objects_)
    {
        object->ForEachComponent<IPreRenderable>([&](IPreRenderable& component)
        {
            component.PreRender(camera, object->GetWorldTransform());
        });
    }
}

void Scene::RegisterObject(Object& object)
{
    object.transform_index_ = static_cast<uint32_t>(ordered_objects_.size());
    ordered_objects_.push_back(&object);
}

void Scene::UnregisterSubtree(Object& object)
{
    for (auto& child : object.children_)
    {
        UnregisterSubtree(*child);
    }

    uint32_t index = object.transform_index_;
    uint32_t last_index = static_cast<uint32_t>(ordered_objects_.size() - 1);
    if (index != last_index)
    {
        ordered_objects_[index] = ordered_objects_[last_index];
        ordered_objects_[index]->transform_index_ = index;
    }
    ordered_objects_.pop_back();
    object.transform_index_ = kInvalidTransformIndex;
}
