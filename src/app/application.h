#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include "app/common/trinity_app.h"
#include "core/scene/camera.h"

#include <unordered_map>
class Entity;
class Renderer;
namespace ecs { class Registry; using EntityId = uint32_t; }

class Application : public ITrinityApp
{
public:
    Application(const std::string& plyFile);
    virtual ~Application() override;

    void OnInitialize() override;
    void OnDrawFrame() override;
    void OnCleanup() override;

    Entity* GetRootEntity() { return root_entity_.get(); }

    Renderer* GetRenderer() { return renderer_.get(); }
    Camera& GetCamera() { return camera_; }
    ecs::Registry& GetRegistry() { return *registry_; }

    void RegisterEntity(ecs::EntityId id, Entity* entity) { entity_map_[id] = entity; }
    void UnregisterEntity(ecs::EntityId id) { entity_map_.erase(id); }
    Entity* GetEntity(ecs::EntityId id)
    {
        auto it = entity_map_.find(id);
        return it != entity_map_.end() ? it->second : nullptr;
    }


#if defined(__ANDROID__)
    void OnSurfaceChanged() override;
#endif

    void ProcessInput(const Uint8* state, float deltaTime);
    void ProcessMouseMotion(float xrel, float yrel);

private:
    void UpdateEntities(float delta_time);

    std::string ply_file_;
    Camera camera_;

    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<ecs::Registry> registry_;
    std::unique_ptr<Entity> root_entity_;
    std::unordered_map<ecs::EntityId, Entity*> entity_map_;

    bool updating_entities_ = false;

    float width_ = 1280.0f;
    float height_ = 720.0f;
};
