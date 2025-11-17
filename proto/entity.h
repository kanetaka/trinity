#pragma once

#include <vector>
#include "vector.h"
#include "matrix.h"
#include "quaternion.h"
#include <cstdint>

class Visualizer;
class Component;

class Entity
{
public:
    enum EntityState
    {
        EActive,
        EPaused,
        EDead
    };

    Entity(Visualizer* game);
    virtual ~Entity();

public:
    void Update(float delta_time);
    void UpdateComponents(float delta_time);
    virtual void UpdateEntity(float delta_time);
    void ProcessInput(const uint8_t* key_state);
    virtual void EntityInput(const uint8_t* key_state);

    const Vec3f& GetPosition() const { return position_; }
    void SetPosition(const Vec3f& pos) { position_ = pos; recompute_world_transform_ = true; }
    float GetScale() const { return scale_; }
    void SetScale(float scale) { scale_ = scale;  recompute_world_transform_ = true; }
    const Quatf GetRotation() const { return rotation_; }
    void SetRotation(const Quatf& rotation) { rotation_ = rotation;  recompute_world_transform_ = true; }

    void ComputeWorldTransform();
    const Mat4f& GetWorldTransform() const { return world_transform_; }

    Vec3f GetForward() { return Vec3f::Transform(rotation_, Vec3f::UNIT_Y); } // TODO 前方方向を決める(right-hand, z-up)

    EntityState GetState() const { return state_; }
    void SetState(EntityState state) { state_ = state; }

    Visualizer* GetVisualizer() { return vis_; }

    void AddComponent(Component* component);
    void RemoveComponent(Component* component);

private:
    EntityState state_;

    Mat4f world_transform_;
    Vec3f position_;
    Quatf rotation_;
    float scale_;
    bool recompute_world_transform_;

    std::vector<Component*> components_;
    Visualizer* vis_;
};
