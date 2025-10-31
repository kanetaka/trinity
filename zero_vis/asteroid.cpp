#include "math.h"
#include "asteroid.h"
#include "sprite_component.h"
#include "move_component.h"
#include "application.h"
#include "random.h"
#include "circle_component.h"

Asteroid::Asteroid(Application* app) :
    Entity(app),
    circle_(nullptr)
{
    Vec2f randPos = Random::GetVector(
            Vec2f(-512.0f, -384.0f), Vec2f(512.0f, 384.0f));
    SetPosition(randPos);
    SetRotation(Random::GetValueRange(0.0f, Math::TWO_PI<float>));

    SpriteComponent* sc = new SpriteComponent(this);
    sc->SetTexture(app->GetTexture("Assets/Asteroid.png"));

    MoveComponent* mc = new MoveComponent(this);
    mc->SetForwardSpeed(150.0f);

    circle_ = new CircleComponent(this);
    circle_->SetRadius(40.0f);

    app->AddAsteroid(this);
}

Asteroid::~Asteroid()
{
    GetApplication()->RemoveAsteroid(this);
}
