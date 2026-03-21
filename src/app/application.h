#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include "app/common/trinity_app.h"
#include "core/camera.h"
#include "core/registry.h"
#include <unordered_map>

namespace tr { 
    class Entity;
    class Registry;
    using EntityId = uint32_t;
    class Renderer; 
    class SplatComponent;
}

class Application : public tr::ITrinityApp
{
public:
    Application(const std::string& plyFile);
    virtual ~Application() override;

    void OnInitialize() override;
    void OnDrawFrame() override;
    void OnCleanup() override;

    tr::Entity* GetRootEntity() { return root_entity_.get(); }

    tr::Renderer* GetRenderer() { return renderer_.get(); }
    tr::Camera& GetCamera() { return camera_; }
    tr::Registry& GetRegistry() { return *registry_; }

    void RegisterEntity(tr::EntityId id, tr::Entity* entity) { entity_map_[id] = entity; }
    void UnregisterEntity(tr::EntityId id) { entity_map_.erase(id); }
    tr::Entity* GetEntity(tr::EntityId id)
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
    std::unique_ptr<tr::SplatComponent> splat_component_;
    tr::Camera camera_;

    std::unique_ptr<tr::Renderer> renderer_;
    std::unique_ptr<tr::Registry> registry_;
    std::unique_ptr<tr::Entity> root_entity_;
    std::unordered_map<tr::EntityId, tr::Entity*> entity_map_;

    bool updating_entities_ = false;

    float width_ = 1280.0f;
    float height_ = 720.0f;
};
