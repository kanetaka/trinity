#pragma once

#include <vector>
#include <string>

class Mesh
{
public:
    Mesh();
    ~Mesh();

    bool Load(const std::string& filename, class Renderer* renderer);
    void Unload();
    class VertexArray* GetVertexArray() { return vertex_array_; }
    class Texture* GetTexture(size_t index);
    const std::string& GetShaderName() const { return shader_name_; }
    float GetRadius() const { return radius_; }
    float GetSpecularPower() const { return specular_power_;  }
private:
    std::vector<class Texture*> textures_;
    class VertexArray* vertex_array_;
    std::string shader_name_;
    float radius_;
    float specular_power_;
};
