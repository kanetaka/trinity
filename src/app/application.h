#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include "app/common/trinity_app.h"
#include "core/camera.h"
#include "core/entity.h"
#include "render/components/splat_component.h"
#include "core/ecs/registry.h"
#include "core/ecs/components.h"

#include <unordered_map>

namespace trinity::render { class Renderer; }
namespace trinity::core::ecs { class Registry; using EntityId = uint32_t; }

class Application : public ITrinityApp
{
public:
    Application(const std::string& plyFile);
    virtual ~Application() override;

    void OnInitialize() override;
    void OnDrawFrame() override;
    void OnCleanup() override;

    trinity::core::Entity* GetRootEntity() { return root_entity_.get(); }

    trinity::render::Renderer* GetRenderer() { return renderer_.get(); }
    trinity::core::Camera& GetCamera() { return camera_; }
    trinity::core::ecs::Registry& GetRegistry() { return *registry_; }

    void RegisterEntity(trinity::core::ecs::EntityId id, trinity::core::Entity* entity) { entity_map_[id] = entity; }
    void UnregisterEntity(trinity::core::ecs::EntityId id) { entity_map_.erase(id); }
    trinity::core::Entity* GetEntity(trinity::core::ecs::EntityId id)
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
    std::unique_ptr<trinity::render::SplatComponent> splat_component_;
    trinity::core::Camera camera_;

    std::unique_ptr<trinity::render::Renderer> renderer_;
    std::unique_ptr<trinity::core::ecs::Registry> registry_;
    std::unique_ptr<trinity::core::Entity> root_entity_;
    std::unordered_map<trinity::core::ecs::EntityId, trinity::core::Entity*> entity_map_;

    bool updating_entities_ = false;

    float width_ = 1280.0f;
    float height_ = 720.0f;
};
