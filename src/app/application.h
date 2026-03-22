#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "app/common/trinity_app.h"
#include "core/camera.h"
#include "core/registry.h"
#include <unordered_map>

namespace tri
{
    class Entity;
    class Registry;
    using EntityId = uint32_t;
    class Renderer;
    class SplatComponent;

    class Application : public ITrinityApp
    {
    public:
        Application(const std::string& plyFile);
        virtual ~Application() override;

        void OnInitialize() override;
        void OnDrawFrame() override;
        void OnCleanup() override;

        static int Run(const std::string& plyFile);

        tri::Entity* GetRootEntity() { return root_entity_.get(); }

        tri::Renderer* GetRenderer() { return renderer_.get(); }
        tri::Camera& GetCamera() { return camera_; }
        tri::Registry& GetRegistry() { return *registry_; }

        void RegisterEntity(tri::EntityId id, tri::Entity* entity) { entity_map_[id] = entity; }
        void UnregisterEntity(tri::EntityId id) { entity_map_.erase(id); }
        tri::Entity* GetEntity(tri::EntityId id)
        {
            auto it = entity_map_.find(id);
            return it != entity_map_.end() ? it->second : nullptr;
        }


#if defined(__ANDROID__)
        void OnSurfaceChanged() override;
#endif

        void ProcessInput(const Uint8* state, float deltaTime);
        void ProcessMouseMotion(float xrel, float yrel);
        void ProcessMouseScroll(float yoffset);
        void ProcessMousePanning(float xrel, float yrel);

    private:
        void UpdateEntities(float delta_time);

        std::string ply_file_;
        std::unique_ptr<tri::SplatComponent> splat_component_;
        tri::Camera camera_;

        std::unique_ptr<tri::Renderer> renderer_;
        std::unique_ptr<tri::Registry> registry_;
        std::unique_ptr<tri::Entity> root_entity_;
        std::unordered_map<tri::EntityId, tri::Entity*> entity_map_;

        bool updating_entities_ = false;

        float width_ = 1280.0f;
        float height_ = 720.0f;
    };
} // namespace tri
