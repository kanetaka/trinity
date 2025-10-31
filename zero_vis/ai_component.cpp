#include "ai_component.h"
#include "entity.h"
#include "ai_state.h"
#include <SDL/SDL_log.h>

AiComponent::AiComponent(class Entity* owner)
    : Component(owner), current_state_(nullptr)
{
}

void AiComponent::Update(float delta_time)
{
    if (current_state_) {
        current_state_->Update(delta_time);
    }
}

void AiComponent::ChangeState(const std::string& name)
{
    if (current_state_) {
        current_state_->OnExit();
    }

    auto iter = state_map_.find(name);
    if (iter != state_map_.end()) {
        current_state_ = iter->second;
        current_state_->OnEnter();
    }
    else {
        SDL_Log("AiState %s の状態はありません", name.c_str());
        current_state_ = nullptr;
    }
}

void AiComponent::RegisterState(class AiState* state)
{
    state_map_.emplace(state->GetName(), state);
}
