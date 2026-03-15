#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include "common/trinity_app.h"
#include "core/camera.h"

class Entity;
class Renderer;

class Application : public ITrinityApp
{
public:
    Application(const std::string &plyFile);
    virtual ~Application() override;

    void OnInitialize() override;
    void OnDrawFrame() override;
    void OnCleanup() override;

    Entity* GetRootEntity() { return root_entity_.get(); }

    Renderer* GetRenderer() { return renderer_.get(); }
    Camera& GetCamera() { return camera_; }

#if defined(__ANDROID__)
    void OnSurfaceChanged() override;
#endif

    void ProcessInput(const Uint8 *state, float deltaTime);
    void ProcessMouseMotion(float xrel, float yrel);

private:
    void UpdateEntities(float delta_time);

    std::string ply_file_;
    Camera camera_;

    std::unique_ptr<Renderer> renderer_;
    std::unique_ptr<Entity> root_entity_;

    bool updating_entities_ = false;

    float width_ = 1280.0f;
    float height_ = 720.0f;
};
