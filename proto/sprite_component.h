#pragma once

#include "component.h"
#include "SDL/SDL.h"

class Entity;
class Texture;

class SpriteComponent : public Component
{
public:
    SpriteComponent(Entity* owner, int draw_order = 100);
    ~SpriteComponent();

    virtual void Draw(class Shader* shader);
    virtual void SetTexture(class Texture* texture);

    int GetDrawOrder() const { return draw_order_; }
    int GetTexHeight() const { return tex_height_; }
    int GetTexWidth() const { return tex_width_; }

protected:
    Texture* texture_;
    int draw_order_;
    int tex_width_;
    int tex_height_;
};
