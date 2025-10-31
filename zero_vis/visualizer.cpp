#include "visualizer.h"
#include <algorithm>
#include "renderer.h"
#include "entity.h"
#include "sprite_component.h"
#include "mesh_component.h"
#include "camera_entity.h"
#include "plane_entity.h"

Visualizer::Visualizer() :
    renderer_(nullptr),
    is_running_(true),
    updating_entity_(false)
{
}

bool Visualizer::Initialize()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
        SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
        return false;
    }

    // Create the renderer
    renderer_ = new Renderer(this);
    if (!renderer_->Initialize(1024.0f, 768.0f)) {
        SDL_Log("Failed to initialize renderer");
        delete renderer_;
        renderer_ = nullptr;
        return false;
    }

    LoadData();

    ticks_count_ = SDL_GetTicks();

    return true;
}

void Visualizer::RunLoop()
{
    while (is_running_) {
        ProcessInput();
        UpdateVisualizer();
        GenerateOutput();
    }
}

void Visualizer::ProcessInput() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_QUIT:
            is_running_ = false;
        break;
        }
    }

    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    if (key_state[SDL_SCANCODE_ESCAPE]) {
        is_running_ = false;
    }

    for (auto entity : entities_) {
        entity->ProcessInput(key_state);
    }
}

void Visualizer::UpdateVisualizer() {
    while (!SDL_TICKS_PASSED(SDL_GetTicks(), ticks_count_ + 16))
        ;

    float delta_time = (SDL_GetTicks() - ticks_count_) / 1000.0f;
    if (delta_time > 0.05f) {
        delta_time = 0.05f;
    }
    ticks_count_ = SDL_GetTicks();

    updating_entity_ = true;
    for (auto entity : entities_) {
        entity->Update(delta_time);
    }
    updating_entity_ = false;

    for (auto pending : pending_entities_) {
        pending->ComputeWorldTransform();
        entities_.emplace_back(pending);
    }
    pending_entities_.clear();

    std::vector<Entity*> dead_entities;
    for (auto entity : entities_) {
        if (entity->GetState() == Entity::EDead) {
            dead_entities.emplace_back(entity);
        }
    }

    for (auto entity : dead_entities) {
        delete entity;
    }
}

void Visualizer::GenerateOutput()
{
    renderer_->Draw();
}

void Visualizer::LoadData()
{
    // Create Entity
    Entity* entity = new Entity(this);
    entity->SetPosition(Vec3f(75.0f, 200.0f, 0.0f));
    entity->SetScale(100.0f);

    Quatf q(Vec3f::UNIT_X, -Math::PI_OVER2<float>); // TODO check
    q = Quatf::Concatenate(q, Quatf(Vec3f::UNIT_Z, Math::PI<float> + Math::PI<float> / 4.0f)); // TODO check
    entity->SetRotation(q);
    MeshComponent* mc = new MeshComponent(entity);
    mc->SetMesh(renderer_->GetMesh("assets/cube.gpmesh"));

#if 0
    entity = new Entity(this);
    entity->SetPosition(Vec3f(200.0f, -75.0f, 0.0f));
    entity->SetScale(3.0f);
    mc = new MeshComponent(entity);
    mc->SetMesh(renderer_->GetMesh("assets/sphere.gpmesh"));

    // Setup floor
    const float start = -1250.0f;
    const float size = 250.0f;
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            entity = new PlaneEntity(this);
            entity->SetPosition(Vec3f(start + i * size, start + j * size, -100.0f));
        }
    }

    // Left/right walls
    q = Quatf(Vec3f::UNIT_X, Math::PI_OVER2<float>); // TODO check
    for (int i = 0; i < 10; i++) {
        entity = new PlaneEntity(this);
        entity->SetPosition(Vec3f(start + i * size, start - size, 0.0f));
        entity->SetRotation(q);

        entity = new PlaneEntity(this);
        entity->SetPosition(Vec3f(start + i * size, -start + size, 0.0f));
        entity->SetRotation(q);
    }

    q = Quatf::Concatenate(q, Quatf(Vec3f::UNIT_Z, Math::PI_OVER2<float>)); // TODO check
    // Forward/back walls
    for (int i = 0; i < 10; i++) {
        entity = new PlaneEntity(this);
        entity->SetPosition(Vec3f(start - size, start + i * size, 0.0f));
        entity->SetRotation(q);

        entity = new PlaneEntity(this);
        entity->SetPosition(Vec3f(-start + size, start + i * size, 0.0f));
        entity->SetRotation(q);
    }
#endif

    // Setup lights
    renderer_->SetAmbientLight(Vec3f(0.2f, 0.2f, 0.2f));
    DirectionalLight& dir = renderer_->GetDirectionalLight();
    dir.direction_ = Vec3f(0.0f, -0.707f, -0.707f);
    dir.diffuse_ = Vec3f(0.78f, 0.88f, 1.0f);
    dir.specular_ = Vec3f(0.8f, 0.8f, 0.8f);

    // Camera actor
     camera_entity_ = new CameraEntity(this);

    // UI elements
    entity = new Entity(this);
    entity->SetPosition(Vec3f(-350.0f, -350.0f, 0.0f));
    SpriteComponent* sc = new SpriteComponent(entity);
    sc->SetTexture(renderer_->GetTexture("assets/health_bar.png"));

    entity = new Entity(this);
    entity->SetPosition(Vec3f(375.0f, -275.0f, 0.0f));
    entity->SetScale(0.75f);
    sc = new SpriteComponent(entity);
    sc->SetTexture(renderer_->GetTexture("assets/radar.png"));
}

void Visualizer::UnloadData()
{
    while (!entities_.empty()) {
        delete entities_.back();
    }

    if (renderer_) {
        renderer_->UnloadData();
    }
}

void Visualizer::Shutdown()
{
    UnloadData();
    if (renderer_) {
        renderer_->Shutdown();
    }
    SDL_Quit();
}

void Visualizer::AddEntity(Entity* entity)
{
    if (updating_entity_) {
        pending_entities_.emplace_back(entity);
    }
    else {
        entities_.emplace_back(entity);
    }
}

void Visualizer::RemoveEntity(Entity* entity)
{
    auto iter = std::find(pending_entities_.begin(), pending_entities_.end(), entity);
    if (iter != pending_entities_.end()) {
        std::iter_swap(iter, pending_entities_.end() - 1);
        pending_entities_.pop_back();
    }

    iter = std::find(entities_.begin(), entities_.end(), entity);
    if (iter != entities_.end()) {
        std::iter_swap(iter, entities_.end() - 1);
        entities_.pop_back();
    }
}
