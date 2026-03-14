#pragma once
#include <vulkan/vulkan.h>
#include <SDL3/SDL.h>
#include <memory>
#include <vector>
#include <string>
#include <chrono>
#include "common/trinity_app.h"
#include "core/camera.h"

class Actor;
class Renderer;

class GsApp : public ITrinityApp {
public:
    GsApp(const std::string &plyFile);
    virtual ~GsApp() override;

    void OnInitialize() override;
    void OnDrawFrame() override;
    void OnCleanup() override;

    void AddActor(Actor* actor);
    void RemoveActor(Actor* actor);

    Renderer* GetRenderer() { return renderer_.get(); }
    Camera& GetCamera() { return camera_; }

#if defined(__ANDROID__)
    void OnSurfaceChanged() override;
#endif

    void ProcessInput(const Uint8 *state, float deltaTime);
    void ProcessMouseMotion(float xrel, float yrel);

private:
    void UpdateActors(float delta_time);

    std::string ply_file_;
    Camera camera_;

    std::unique_ptr<Renderer> renderer_;
    std::vector<Actor*> actors_;
    std::vector<Actor*> pending_actors_;

    bool updating_actors_ = false;

    float width_ = 1280.0f;
    float height_ = 720.0f;
};
