#include "sprite_component.h"
#include "texture.h"
#include "shader.h"
#include "entity.h"
#include "visualizer.h"
#include "renderer.h"

SpriteComponent::SpriteComponent(Entity* owner, int draw_order):
    Component(owner),
    texture_(nullptr),
    draw_order_(draw_order),
    tex_width_(0),
    tex_height_(0)
{
    owner_->GetVisualizer()->GetRenderer()->AddSprite(this);
}

SpriteComponent::~SpriteComponent()
{
    owner_->GetVisualizer()->GetRenderer()->RemoveSprite(this);
}

void SpriteComponent::Draw(Shader* shader)
{
    if (texture_) {
        Mat4f scale_mat = Mat4f::CreateScale(
                static_cast<float>(tex_width_),
                static_cast<float>(tex_height_),
                1.0f);

        Mat4f world = owner_->GetWorldTransform() * scale_mat; // col-vertor
        shader->SetMatrixUniform("uWorldTransform", world);
        texture_->SetActive();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
    }
}

void SpriteComponent::SetTexture(Texture* texture)
{
    texture_ = texture;
    tex_width_ = texture->GetWidth();
    tex_height_ = texture->GetHeight();
}
