#include "laser.h"
#include "sprite_component.h"
#include "move_component.h"
#include "application.h"
#include "circle_component.h"
#include "asteroid.h"

Laser::Laser(Application* app) :
    Entity(app),
    death_timer_(1.0f)
{
    SpriteComponent* sc = new SpriteComponent(this);
    sc->SetTexture(app->GetTexture("Assets/Laser.png"));

    MoveComponent* mc = new MoveComponent(this);
    mc->SetForwardSpeed(800.0f);

    circle_ = new CircleComponent(this);
    circle_->SetRadius(11.0f);
}

void Laser::UpdateEntity(float delta_time)
{
    death_timer_ -= delta_time;
    if (death_timer_ <= 0.0f) {
        SetState(EDead);
    }
    else {
        for (auto ast : GetApplication()->GetAsteroids()) {
            if (Intersect(*circle_, *(ast->GetCircle()))) {
                SetState(EDead);
                ast->SetState(EDead);
                break;
            }
        }
    }
}
