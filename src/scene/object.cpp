#include "scene/object.h"
#include "scene/scene.h"
#include <glm/gtc/matrix_transform.hpp>

using namespace tri;

Object::Object(std::string name)
    : name_(std::move(name))
{
}

Object::~Object() = default;

void Object::OnComponentAdded(IComponent& component)
{
    if (scene_)
    {
        scene_->RegisterComponent(*this, component);
    }
}

void Object::OnComponentRemoved(IComponent& component)
{
    if (scene_)
    {
        scene_->UnregisterComponent(*this, component);
    }
}

void Object::SetLocalPosition(const glm::dvec3& position)
{
    local_position_ = position;
    transform_dirty_ = true;
}

void Object::SetLocalRotation(const glm::quat& rotation)
{
    local_rotation_ = rotation;
    transform_dirty_ = true;
}

void Object::SetLocalScale(float scale)
{
    local_scale_ = scale;
    transform_dirty_ = true;
}

void Object::UpdateWorldTransform(const glm::dmat4& parent_world)
{
    if (transform_dirty_)
    {
        local_transform_ = glm::translate(glm::dmat4(1.0), local_position_);
        local_transform_ *= glm::dmat4(glm::mat4_cast(local_rotation_));
        local_transform_ = glm::scale(local_transform_, glm::dvec3(local_scale_));
        transform_dirty_ = false;
    }

    world_transform_ = parent_world * local_transform_;

    for (auto& child : children_)
    {
        child->UpdateWorldTransform(world_transform_);
    }
}
