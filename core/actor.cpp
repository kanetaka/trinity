#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif

#include "actor.h"
#include "component.h"
#include "gs_app.h"
#include "core/renderer.h"
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <vector>
#include <cstdint>

// Forward declarations
class GsApp;
class Component;

Actor::Actor(GsApp* game)
        : state_(EActive), position_(0.0f, 0.0f, 0.0f), rotation_(glm::identity<glm::quat>()),
            scale_(1.0f), recompute_world_transform_(true), game_(game) {
    // game_->AddActor(this); // This should be called by GsApp or similar
}

Actor::~Actor() {
    // game_->RemoveActor(this);
    while (!components_.empty()) {
        delete components_.back();
    }
}

void Actor::Update(float delta_time) {
    if (state_ == EActive) {
        ComputeWorldTransform();
        UpdateComponents(delta_time);
        UpdateActor(delta_time);
        ComputeWorldTransform();
    }
}

void Actor::UpdateComponents(float delta_time) {
    for (auto comp : components_) {
        comp->Update(delta_time);
    }
}

void Actor::UpdateActor(float delta_time) {}

void Actor::ProcessInput(const uint8_t* key_state) {
    if (state_ == EActive) {
        for (auto comp : components_) {
            comp->ProcessInput(key_state);
        }
        ActorInput(key_state);
    }
}

void Actor::ActorInput(const uint8_t* key_state) {}

void Actor::ComputeWorldTransform() {
    if (recompute_world_transform_) {
        recompute_world_transform_ = false;
        // Scale, then rotate, then translate
        world_transform_ = glm::translate(glm::mat4(1.0f), position_);
        world_transform_ *= glm::mat4_cast(rotation_);
        world_transform_ = glm::scale(world_transform_, glm::vec3(scale_));

        // Inform components world transform updated
        for (auto comp : components_) {
            comp->OnUpdateWorldTransform();
        }
    }
}

void Actor::AddComponent(Component* component) {
    // Find the insertion point before the first element with a higher update order
    int my_order = component->GetUpdateOrder();
    auto iter = components_.begin();
    for (; iter != components_.end(); ++iter) {
        if (my_order < (*iter)->GetUpdateOrder()) {
            break;
        }
    }
    components_.insert(iter, component);
}

void Actor::RemoveComponent(Component* component) {
    auto iter = std::find(components_.begin(), components_.end(), component);
    if (iter != components_.end()) {
        components_.erase(iter);
    }
}
