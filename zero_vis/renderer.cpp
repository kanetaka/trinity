#include "renderer.h"
#include "texture.h"
#include "mesh.h"
#include <algorithm>
#include "shader.h"
#include "vertex_array.h"
#include "sprite_component.h"
#include "mesh_component.h"
#include "GL/glew.h"

Renderer::Renderer(class Visualizer* vis) : 
    aplication_(vis),
    sprite_shader_(nullptr),
    mesh_shader_(nullptr)
{
}

Renderer::~Renderer()
{
}

bool Renderer::Initialize(float screen_width, float screen_height)
{
    screen_width_ = screen_width;
    screen_height_ = screen_height;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    // OpenGL version 3.3
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

    window_ = SDL_CreateWindow(
            "ZeroVis",
            100, 100,
            static_cast<int>(screen_width), static_cast<int>(screen_height),
            SDL_WINDOW_OPENGL);

    if (!window_) {
        SDL_Log("Failed to create window: %s", SDL_GetError());
        return false;
    }

    context_ = SDL_GL_CreateContext(window_);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        SDL_Log("Failed to initialize GLEW.");
        return false;
    }
    glGetError();

    if (!LoadShaders()) {
        SDL_Log("Failed to load shaders");
        return false;
    }

    CreateSpriteVerts();

    return true;
}

void Renderer::Shutdown()
{
    delete sprite_verts_;

    sprite_shader_->Unload();
    delete sprite_shader_;

    mesh_shader_->Unload();
    delete mesh_shader_;

    SDL_GL_DeleteContext(context_);
    SDL_DestroyWindow(window_);
}

void Renderer::UnloadData()
{
    for (auto t : textures_) {
        t.second->Unload();
        delete t.second;
    }
    textures_.clear();

    for (auto m : meshes_) {
        m.second->Unload();
        delete m.second;
    }

    meshes_.clear();
}

void Renderer::Draw()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    mesh_shader_->SetActive();
    mesh_shader_->SetMatrixUniform("uViewProj", projection_ * view_);

    SetLightUniforms(mesh_shader_);
    for (auto mc : mesh_component_) {
        mc->Draw(mesh_shader_);
    }

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    sprite_shader_->SetActive();
    sprite_verts_->SetActive();

    for (auto sprite : sprites_) {
        sprite->Draw(sprite_shader_);
    }

    SDL_GL_SwapWindow(window_);
}

void Renderer::AddSprite(SpriteComponent* sprite)
{
    int my_draw_order = sprite->GetDrawOrder();
    auto iter = sprites_.begin();

    for (; iter != sprites_.end(); ++iter) {
        if (my_draw_order < (*iter)->GetDrawOrder()) {
            break;
        }
    }
    sprites_.insert(iter, sprite);
}

void Renderer::RemoveSprite(SpriteComponent* sprite)
{
    auto iter = std::find(sprites_.begin(), sprites_.end(), sprite);
    sprites_.erase(iter);
}

void Renderer::AddMeshComponent(MeshComponent* mesh)
{
    mesh_component_.emplace_back(mesh);
}

void Renderer::RemoveMeshComponent(MeshComponent* mesh)
{
    auto iter = std::find(mesh_component_.begin(), mesh_component_.end(), mesh);
    mesh_component_.erase(iter);
}

Texture* Renderer::GetTexture(const std::string& file_name)
{
    Texture* tex = nullptr;
    auto iter = textures_.find(file_name);
    if (iter != textures_.end()) {
        tex = iter->second;
    }
    else {
        tex = new Texture();
        if (tex->Load(file_name)) {
            textures_.emplace(file_name, tex);
        }
        else {
            delete tex;
            tex = nullptr;
        }
    }

    return tex;
}

Mesh* Renderer::GetMesh(const std::string& file_name)
{
    Mesh* mesh = nullptr;
    auto iter = meshes_.find(file_name);

    if (iter != meshes_.end()) {
        mesh = iter->second;
    }
    else {
        mesh = new Mesh();
        if (mesh->Load(file_name, this)) {
            meshes_.emplace(file_name, mesh);
        }
        else {
            delete mesh;
            mesh = nullptr;
        }
    }

    return mesh;
}

bool Renderer::LoadShaders()
{
    sprite_shader_ = new Shader();
    if (!sprite_shader_->Load("shaders/sprite.vert", "shaders/sprite.frag")) {
        return false;
    }

    sprite_shader_->SetActive();
    Matrix4 view_proj = Mat4f::CreateSimpleViewProj(screen_width_, screen_height_);
    sprite_shader_->SetMatrixUniform("uViewProj", view_proj);

    mesh_shader_ = new Shader();
    if (!mesh_shader_->Load("shaders/phong.vert", "shaders/phong.frag")) {
        return false;
    }

    mesh_shader_->SetActive();
    view_ = Mat4f::CreateLookAt(Vec3f::ZERO, Vec3f::UNIT_Y, Vec3f::UNIT_Z); // TODO check
    projection_ = Mat4f::CreatePerspectiveFov(Math::ToRadians(70.0f),
            screen_width_, screen_height_, 25.0f, 10000.0f);
    mesh_shader_->SetMatrixUniform("uViewProj", projection_ * view_); // TODO check

    return true;
}

void Renderer::CreateSpriteVerts()
{
    float vertices[] = {
        -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, // Top Left
         0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, // Top Right
         0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, // Bottom Right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f  // Bottom Left
    };

    unsigned int indices[] = {
        1, 0, 2,
        2, 0, 3
    };
    sprite_verts_ = new VertexArray(vertices, 4, indices, 6);
}

void Renderer::SetLightUniforms(Shader* shader)
{
    Mat4f inv_view = view_;

    inv_view.Invert();
    shader->SetVectorUniform("uCameraPos", inv_view.GetTranslation());
    shader->SetVectorUniform("uAmbientLight", ambient_light_);
    shader->SetVectorUniform("uDirLight.mDirection", dir_light_.direction_);
    shader->SetVectorUniform("uDirLight.mDiffuseColor", dir_light_.diffuse_);
    shader->SetVectorUniform("uDirLight.mSpecColor", dir_light_.specular_);
}
