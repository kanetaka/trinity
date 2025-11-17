#pragma once

#include "component.h"
#include <unordered_map>
#include <string>

class AiComponent : Component
{
public:
    AiComponent(class Entity* owner);

    void Update(float delta_time) override;
    void ChangeState(const std::string& name);
    void RegisterState(class AiState* state);

private:
    std::unordered_map<std::string, class AiState*> state_map_;
    class AiState* current_state_;
};
