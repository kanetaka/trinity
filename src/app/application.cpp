#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "app/application.h"
#include "core/scene/entity.h"
#include "core/renderer/renderer.h"
#include "components/splat_component.h"
#include "core/graphics/vulkan_context.h"
#include "core/graphics/swapchain.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include "core/ecs/registry.h"
#include "core/ecs/systems.h"
#include "core/ecs/components.h"

Application::Application(const std::string &plyFile)
        : ply_file_(plyFile), camera_(glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f, -1.0f, 0.0f), -90.0f, 0.0f),
            updating_entities_(false)
{
    registry_ = std::make_unique<ecs::Registry>();
}

Application::~Application()
{
}

void Application::OnInitialize()
{
    renderer_ = std::make_unique<Renderer>(this);
    auto extent = VulkanContext::Get().GetSwapchain()->GetExtent();
    renderer_->Initialize((float)extent.width, (float)extent.height);

    root_entity_ = std::make_unique<Entity>(*registry_, registry_->Create());
    RegisterEntity(root_entity_->GetId(), root_entity_.get());

    // Create Splat Entity as a child of root
    ecs::EntityId splat_id = registry_->Create();
    Entity* splat_entity = new Entity(*registry_, splat_id);
    RegisterEntity(splat_id, splat_entity);
    root_entity_->AddChild(splat_id);
    new SplatComponent(splat_entity, ply_file_, renderer_.get());
}

void Application::OnCleanup()
{
    if (renderer_)
    {
        renderer_->Shutdown();
        renderer_.reset();
    }
    root_entity_.reset();
}

void Application::OnDrawFrame()
{
    static auto lastTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::chrono::seconds::period> diff = currentTime - lastTime;
    float deltaTime = diff.count();
    lastTime = currentTime;

    // Update Renderer matrices
    renderer_->SetViewMatrix(camera_.GetViewMatrix());
    renderer_->SetProjectionMatrix(camera_.GetProjectionMatrix(width_ / height_));
    renderer_->UpdateUniformBuffer();

    ecs::TransformSystem::Update(*registry_);

    if (root_entity_)
    {
        root_entity_->Update(deltaTime);
        // Find SplatComponent and call UpdateWithCamera
        for (auto child_id : root_entity_->GetChildren()) {
            if (auto* child = GetEntity(child_id)) {
                for (auto comp : child->GetComponents()) {
                    if (auto* splat = dynamic_cast<SplatComponent*>(comp)) {
                        splat->UpdateWithCamera(deltaTime, camera_);
                    }
                }
            }
        }
    }

    renderer_->Draw(root_entity_.get());
}

void Application::ProcessInput(const Uint8 *state, float deltaTime)
{
    camera_.ProcessKeyboard(state, deltaTime);
    if (root_entity_)
    {
        root_entity_->ProcessInput(state);
    }
}

void Application::ProcessMouseMotion(float xrel, float yrel)
{
    camera_.ProcessMouseMovement(xrel, -yrel); // Invert y
}

#if defined(__ANDROID__)
void Application::OnSurfaceChanged()
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
