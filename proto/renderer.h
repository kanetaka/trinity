#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <SDL/SDL.h>
#include "vector.h"

class Visualizer;
class SpriteComponent;
class Mesh;
class MeshComponent;
class Texture;
class VertexArray;
class Shader;

struct DirectionalLight
{
    Vec3f direction_;
    Vec3f diffuse_;
    Vec3f specular_;
};

class Renderer
{
public:
    Renderer(Visualizer* vis);
    ~Renderer();

    bool Initialize(float screen_width, float screen_height);
    void Shutdown();
    void UnloadData();

    void Draw();

    void AddSprite(SpriteComponent* sprite);
    void RemoveSprite(SpriteComponent* sprite);

    void AddMeshComponent(MeshComponent* mesh);
    void RemoveMeshComponent(MeshComponent* mesh);

    Texture* GetTexture(const std::string& file_name);
    Mesh* GetMesh(const std::string& file_name);

    void SetViewMatrix(const Mat4f& view) { view_ = view; }

    void SetAmbientLight(const Vec3f& ambient) { ambient_light_ = ambient; }
    DirectionalLight& GetDirectionalLight() { return dir_light_; }

    float GetScreenWidth() const { return screen_width_; }
    float GetScreenHeight() const { return screen_height_; }

private:
    bool LoadShaders();
    void CreateSpriteVerts();
    void SetLightUniforms(Shader* shader);

private:
    std::unordered_map<std::string, Texture*> textures_;
    std::unordered_map<std::string, Mesh*> meshes_;
    std::vector<SpriteComponent*> sprites_;
    std::vector<MeshComponent*> mesh_component_;

    Visualizer* aplication_;
    Shader* sprite_shader_;
    VertexArray* sprite_verts_;
    Shader* mesh_shader_;

    Mat4f view_;
    Mat4f projection_;
    float screen_width_;
    float screen_height_;

    Vec3f ambient_light_;
    DirectionalLight dir_light_;

    SDL_Window* window_;
    SDL_GLContext context_;
};
