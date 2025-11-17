#pragma once
#include "component.h"
#include <cstddef>

class Shader;
class Mesh;

class MeshComponent : public Component
{
public:
    MeshComponent(class Entity* owner);
    ~MeshComponent();

    virtual void Draw(Shader* shader);
    virtual void SetMesh(Mesh* mesh) { mesh_ = mesh; }
    void SetTextureIndex(size_t index) { texture_index_ = index; }

protected:
    Mesh* mesh_;
    size_t texture_index_;
};
