#pragma once
#include "entity.h"

class CameraEntity : public Entity
{
public:
    CameraEntity(class Visualizer* vis);

    void UpdateEntity(float delta_time) override;
    void EntityInput(const uint8_t* keys) override;
private:
    class MoveComponent* move_comp_;
};
