#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "app/application.h"
#include "core/entity.h"
#include "render/renderer.h"
#include "render/components/splat_component.h"
#include "render/vulkan_context.h"
#include "render/swapchain.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include "core/ecs/registry.h"
#include "core/ecs/systems.h"
#include "core/ecs/components.h"

using namespace trinity::core;
using namespace trinity::render;

Application::Application(const std::string& plyFile)
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
    trinity::core::Entity* splat_entity = new Entity(*registry_, splat_id);
    RegisterEntity(splat_id, splat_entity);
    root_entity_->AddChild(splat_id);

    // Use member splat_component_ to initialize ECS component
    splat_component_ = std::make_unique<SplatComponent>(ply_file_, renderer_.get());
    splat_component_->Initialize(*registry_, splat_id, renderer_.get());
}

void Application::OnCleanup()
{
    // Vulkanリソースはデバイス破棄前に解放する必要がある
    splat_component_.reset();
    entity_map_.clear();
    root_entity_.reset();
    registry_.reset();

    if (renderer_)
    {
        renderer_->Shutdown();
        renderer_.reset();
    }
}

void Application::OnDrawFrame()
{
    static auto last_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::chrono::seconds::period> diff = current_time - last_time;
    float delta_time = diff.count();
    last_time = current_time;

    // Update Renderer matrices
    renderer_->SetViewMatrix(camera_.GetViewMatrix());
    renderer_->SetProjectionMatrix(camera_.GetProjectionMatrix(width_ / height_));
    renderer_->UpdateUniformBuffer();

    ecs::TransformSystem::Update(*registry_);
    renderer_->UpdateTransformBuffer(*registry_);

    if (root_entity_)
    {
        root_entity_->Update(delta_time);
        
        // Use member splat_component_ for specific logic (like sorting)
        if (splat_component_)
        {
            // We need the entity ID for the splat data. 
            // Looking up by name or just keeping the ID in application.
            // For now, let's find it or use a known ID if we stored it.
            // Assuming the last created splat_id in OnInitialize was what we want.
            // To be safe, we can iterate entities or store the ID.
            // Let's assume we want to update all SplatDataComponents.
            registry_->ForEach<trinity::core::ecs::SplatDataComponent>([&](trinity::core::ecs::EntityId entity, trinity::core::ecs::SplatDataComponent& data) {
                splat_component_->UpdateWithCamera(*registry_, entity, camera_);
            });
        }
    }

    renderer_->Draw(root_entity_.get());
}

void Application::ProcessInput(const Uint8* state, float delta_time)
{
    camera_.ProcessKeyboard(state, delta_time);
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
    auto& vulkan_ctx = VulkanContext::Get();
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
