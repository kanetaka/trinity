#include "ai_state.h"
#include "ai_component.h"
#include <SDL/SDL_log.h>

void AiPatrol::Update(float delta_time)
{
    SDL_Log("Updating %s state", GetName());
    bool dead = true;
    if (dead) {
        owner_->ChangeState("Death");
    }
}

void AiPatrol::OnEnter()
{
    SDL_Log("Entering %s state", GetName());
}

void AiPatrol::OnExit()
{
    SDL_Log("Exiting %s state", GetName());
}

void AiDeath::Update(float delta_time)
{
    SDL_Log("Updating %s state", GetName());
    bool dead = true;
    if (dead) {
        owner_->ChangeState("Death");
    }
}

void AiDeath::OnEnter()
{
    SDL_Log("Entering %s state", GetName());
}

void AiDeath::OnExit()
{
    SDL_Log("Exiting %s state", GetName());
}

void AiAttack::Update(float delta_time)
{
    SDL_Log("Updating %s state", GetName());
    bool dead = true;
    if (dead) {
        owner_->ChangeState("Death");
    }
}

void AiAttack::OnEnter()
{
    SDL_Log("Entering ¥s state", GetName());
}

void AiAttack::OnExit()
{
    SDL_Log("Exiting ¥s state", GetName());
}
