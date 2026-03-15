#pragma once
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <cstdint>

class GsApp;
class Component;

class Entity
{
public:
    enum State
    {
        EActive,
        EPaused,
        EDead
    };

    Entity(GsApp* game);
    virtual ~Entity();

    void Update(float delta_time);
    void UpdateComponents(float delta_time);
    virtual void UpdateEntity(float delta_time);

    void ProcessInput(const uint8_t* key_state);
    virtual void EntityInput(const uint8_t* key_state);

    const glm::vec3& GetPosition() const { return position_; }
    void SetPosition(const glm::vec3& pos) { position_ = pos; recompute_world_transform_ = true; }
    float GetScale() const { return scale_; }
    void SetScale(float scale) { scale_ = scale; recompute_world_transform_ = true; }
    const glm::quat& GetRotation() const { return rotation_; }
    void SetRotation(const glm::quat& rotation) { rotation_ = rotation; recompute_world_transform_ = true; }

    void ComputeWorldTransform();
    const glm::mat4& GetWorldTransform() const { return world_transform_; }

    glm::vec3 GetForward() const { return rotation_ * glm::vec3(1.0f, 0.0f, 0.0f); }

    State GetState() const { return state_; }
    void SetState(State state) { state_ = state; }

    GsApp* GetGame() { return game_; }

    void AddComponent(Component* component);
    void RemoveComponent(Component* component);

private:
    State state_;

    glm::mat4 world_transform_;
    glm::vec3 position_;
    glm::quat rotation_;
    float scale_;
    bool recompute_world_transform_;

    std::vector<Component*> components_;
    GsApp* game_;
};
