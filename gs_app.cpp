#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "gs_app.h"
#include "core/actor.h"
#include "core/renderer.h"
#include "core/splat_component.h"
#include "core/vulkan_context.h"
#include "core/swapchain.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>

GsApp::GsApp(const std::string &plyFile)
        : ply_file_(plyFile), camera_(glm::vec3(0.0f, 0.0f, 5.0f),
                                                                     glm::vec3(0.0f, -1.0f, 0.0f), -90.0f, 0.0f),
            updating_actors_(false) {
}

GsApp::~GsApp() {
}

void GsApp::OnInitialize() {
    renderer_ = std::make_unique<Renderer>(this);
    auto extent = VulkanContext::Get().GetSwapchain()->GetExtent();
    renderer_->Initialize((float)extent.width, (float)extent.height);

    // Create Splat Actor
    Actor* splat_actor = new Actor(this);
    AddActor(splat_actor);
    new SplatComponent(splat_actor, ply_file_);
}

void GsApp::OnCleanup() {
    // Delete all actors first, as components may refer to renderer
    while (!actors_.empty()) {
        delete actors_.back();
        actors_.pop_back();
    }
    while (!pending_actors_.empty()) {
        delete pending_actors_.back();
        pending_actors_.pop_back();
    }

    if (renderer_) {
        renderer_->Shutdown();
        renderer_.reset();
    }
}

void GsApp::OnDrawFrame() {
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = currentTime - lastTime;
    float deltaTime = diff.count();
    lastTime = currentTime;

    // Update Renderer matrices
    renderer_->SetViewMatrix(camera_.GetViewMatrix());
    renderer_->SetProjectionMatrix(camera_.GetProjectionMatrix(width_ / height_));
    renderer_->UpdateUniformBuffer();

    UpdateActors(deltaTime);
    renderer_->Draw();
}

void GsApp::UpdateActors(float delta_time) {
    updating_actors_ = true;
    for (auto actor : actors_) {
        actor->Update(delta_time);
    }
    updating_actors_ = false;

    for (auto actor : pending_actors_) {
        actors_.push_back(actor);
    }
    pending_actors_.clear();

    std::vector<Actor*> dead_actors;
    for (auto actor : actors_) {
        if (actor->GetState() == Actor::EDead) {
            dead_actors.push_back(actor);
        }
    }

    for (auto actor : dead_actors) {
        RemoveActor(actor);
        delete actor;
    }
}

void GsApp::AddActor(Actor* actor) {
    if (updating_actors_) {
        pending_actors_.push_back(actor);
    } else {
        actors_.push_back(actor);
    }
}

void GsApp::RemoveActor(Actor* actor) {
    auto iter = std::find(pending_actors_.begin(), pending_actors_.end(), actor);
    if (iter != pending_actors_.end()) {
        pending_actors_.erase(iter);
    }

    iter = std::find(actors_.begin(), actors_.end(), actor);
    if (iter != actors_.end()) {
        actors_.erase(iter);
    }
}

void GsApp::ProcessInput(const Uint8 *state, float deltaTime) {
    camera_.ProcessKeyboard(state, deltaTime);
    // Also pass to actors
    for (auto actor : actors_) {
        actor->ProcessInput(state);
    }
}

void GsApp::ProcessMouseMotion(float xrel, float yrel) {
    camera_.ProcessMouseMovement(xrel, -yrel); // Invert y
}

#if defined(__ANDROID__)
void GsApp::OnSurfaceChanged() {
    auto &vulkan_ctx = VulkanContext::Get();
    vulkan_ctx.RecreateSwapchain();
    auto extent = vulkan_ctx.GetSwapchainExtent();
    width_ = (float)extent.width;
    height_ = (float)extent.height;
    if (renderer_) {
        renderer_->Initialize(width_, height_);
    }
}
#endif
