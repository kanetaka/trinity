#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "gs_app.h"
#include "core/entity.h"
#include "core/renderer.h"
#include "core/splat_component.h"
#include "core/vulkan_context.h"
#include "core/swapchain.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>

GsApp::GsApp(const std::string &plyFile)
        : ply_file_(plyFile), camera_(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, -1.0f, 0.0f), -90.0f, 0.0f),
            updating_entities_(false)
{
}

GsApp::~GsApp()
{
}

void GsApp::OnInitialize()
{
    renderer_ = std::make_unique<Renderer>(this);
    auto extent = VulkanContext::Get().GetSwapchain()->GetExtent();
    renderer_->Initialize((float)extent.width, (float)extent.height);

    // Create Splat Entity
    Entity* splat_entity = new Entity(this);
    AddEntity(splat_entity);
    new SplatComponent(splat_entity, ply_file_);
}

void GsApp::OnCleanup()
{
    // Delete all entities first, as components may refer to renderer
    while (!entities_.empty())
    {
        delete entities_.back();
        entities_.pop_back();
    }
    while (!pending_entities_.empty())
    {
        delete pending_entities_.back();
        pending_entities_.pop_back();
    }

    if (renderer_)
    {
        renderer_->Shutdown();
        renderer_.reset();
    }
}

void GsApp::OnDrawFrame()
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> diff = currentTime - lastTime;
    float deltaTime = diff.count();
    lastTime = currentTime;

    // Update Renderer matrices
    renderer_->SetViewMatrix(camera_.GetViewMatrix());
    renderer_->SetProjectionMatrix(camera_.GetProjectionMatrix(width_ / height_));
    renderer_->UpdateUniformBuffer();

    UpdateEntities(deltaTime);
    renderer_->Draw();
}

void GsApp::UpdateEntities(float delta_time)
{
    updating_entities_ = true;
    for (auto entity : entities_)
    {
        entity->Update(delta_time);
    }
    updating_entities_ = false;

    for (auto entity : pending_entities_)
    {
        entities_.push_back(entity);
    }
    pending_entities_.clear();

    std::vector<Entity*> dead_entities;
    for (auto entity : entities_)
    {
        if (entity->GetState() == Entity::EDead)
        {
            dead_entities.push_back(entity);
        }
    }

    for (auto entity : dead_entities)
    {
        RemoveEntity(entity);
        delete entity;
    }
}

void GsApp::AddEntity(Entity* entity)
{
    if (updating_entities_)
    {
        pending_entities_.push_back(entity);
    }
    else
    {
        entities_.push_back(entity);
    }
}

void GsApp::RemoveEntity(Entity* entity)
{
    auto iter = std::find(pending_entities_.begin(), pending_entities_.end(), entity);
    if (iter != pending_entities_.end())
    {
        pending_entities_.erase(iter);
    }

    iter = std::find(entities_.begin(), entities_.end(), entity);
    if (iter != entities_.end())
    {
        entities_.erase(iter);
    }
}

void GsApp::ProcessInput(const Uint8 *state, float deltaTime)
{
    camera_.ProcessKeyboard(state, deltaTime);
    // Also pass to entities
    for (auto entity : entities_)
    {
        entity->ProcessInput(state);
    }
}

void GsApp::ProcessMouseMotion(float xrel, float yrel)
{
    camera_.ProcessMouseMovement(xrel, -yrel); // Invert y
}

#if defined(__ANDROID__)
void GsApp::OnSurfaceChanged()
{
    auto &vulkan_ctx = VulkanContext::Get();
    vulkan_ctx.RecreateSwapchain();
    auto extent = vulkan_ctx.GetSwapchainExtent();
    width_ = (float)extent.width;
    height_ = (float)extent.height;
    if (renderer_)
    {
        renderer_->Initialize(width_, height_);
    }
}
#endif
