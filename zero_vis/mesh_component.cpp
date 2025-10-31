#include "mesh_component.h"
#include "shader.h"
#include "mesh.h"
#include "entity.h"
#include "visualizer.h"
#include "renderer.h"
#include "texture.h"
#include "vertex_array.h"


MeshComponent::MeshComponent(class Entity* owner) :
    Component(owner),
    mesh_(nullptr),
    texture_index_(0)
{
    owner_->GetVisualizer()->GetRenderer()->AddMeshComponent(this);
}

MeshComponent::~MeshComponent()
{
    owner_->GetVisualizer()->GetRenderer()->RemoveMeshComponent(this);
}

void MeshComponent::Draw(Shader* shader)
{
    if (mesh_) {
        shader->SetMatrixUniform("uWorldTransform", owner_->GetWorldTransform());
        shader->SetFloatUniform("uSpecularPower", mesh_->GetSpecularPower());

        Texture* tex = mesh_->GetTexture(texture_index_);
        if (tex) {
            tex->SetActive();
        }
    }
}
