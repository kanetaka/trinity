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
#include "core/math/camera.h"
#include "core/ecs/registry.h"

namespace tri
{
    class Registry;
    class Renderer;
    class SplatSystem;

    class Application : public ITrinityApp
    {
    public:
        Application(const std::string& plyFile);
        virtual ~Application() override;

        void OnInitialize() override;
        void OnDrawFrame() override;
        void OnCleanup() override;

        static int Run(const std::string& plyFile);

        Entity GetRootEntity() const { return root_entity_; }

        tri::Renderer* GetRenderer() { return renderer_.get(); }
        tri::Camera& GetCamera() { return camera_; }
        tri::Registry& GetRegistry() { return *registry_; }


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
        std::unique_ptr<tri::SplatSystem> splat_system_;
        tri::Camera camera_;

        std::unique_ptr<tri::Renderer> renderer_;
        std::unique_ptr<tri::Registry> registry_;
        Entity root_entity_ = NullEntity;

        bool updating_entities_ = false;

        float width_ = 1280.0f;
        float height_ = 720.0f;
    };
} // namespace tri
