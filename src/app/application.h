#pragma once
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "app/i_application.h"
#include "geometry/camera.h"
#include "ui/ui_manager.h"
#include "scene/scene.h"

namespace tri
{
    class Renderer;
    class GraphicsContext;

    class Application : public IApplication
    {
    public:
        Application();
        virtual ~Application() override;

        void OnInitialize() override;
        void OnDrawFrame() override;
        void OnCleanup() override;

        static int Run(const std::string& json_args);

        void LoadPly(const std::string& path);

        Object& GetRootObject() { return scene_->GetRoot(); }

        tri::Renderer* GetRenderer() { return renderer_.get(); }
        tri::Camera& GetCamera() { return camera_; }
        tri::Scene& GetScene() { return *scene_; }
        tri::UiManager& GetUiManager() { return *ui_manager_; }
        tri::GraphicsContext& GetGraphicsContext() { return *graphics_context_; }
        const tri::GraphicsContext& GetGraphicsContext() const { return *graphics_context_; }


#if defined(__ANDROID__)
        void OnSurfaceChanged() override;
#endif

        void ProcessInput(const Uint8* state, float delta_time);
        void ProcessMouseMotion(float xrel, float yrel);
        void ProcessMouseScroll(float yoffset);
        void ProcessMousePanning(float xrel, float yrel);

    private:
        std::string ply_file_;
        tri::Camera camera_;

        std::unique_ptr<tri::Renderer> renderer_;
        std::unique_ptr<tri::Scene> scene_;
        std::unique_ptr<tri::UiManager> ui_manager_;
        std::unique_ptr<tri::GraphicsContext> graphics_context_;

        float width_ = 1280.0f;
        float height_ = 720.0f;
    };
} // namespace tri
