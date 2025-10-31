#include "ship.h"
#include "sprite_component.h"
#include "input_component.h"
#include "application.h"
#include "laser.h"

Ship::Ship(Application* app) :
    Entity(app),
    laser_cool_down_(0.0f)
{
    SpriteComponent* sc = new SpriteComponent(this, 150);
    sc->SetTexture(app->GetTexture("Assets/Ship.png"));

    InputComponent* ic = new InputComponent(this);
    ic->SetForwardKey(SDL_SCANCODE_W);
    ic->SetBackKey(SDL_SCANCODE_S);
    ic->SetClockwiseKey(SDL_SCANCODE_A);
    ic->SetCounterClockwiseKey(SDL_SCANCODE_D);
    ic->SetMaxForwardSpeed(300.0f);
    ic->SetMaxAngularSpeed(Math::TWO_PI<float>);
}

void Ship::UpdateEntity(float delta_time)
{
    laser_cool_down_ -= delta_time;
}

void Ship::EntityInput(const uint8_t* key_state)
{
    if (key_state[SDL_SCANCODE_SPACE] && laser_cool_down_ <= 0.0f) {
        Laser* laser = new Laser(GetApplication());
        laser->SetPosition(GetPosition());
        laser->SetRotation(GetRotation());
        laser_cool_down_ = 0.5f;
    }
}
