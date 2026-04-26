#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "app/application.h"
#include "render/renderer.h"
#include "render/system/splat_system.h"
#include "render/vulkan_context.h"
#include "render/swapchain.h"
#include "render/surface/sdl3_surface_provider.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include "core/registry.h"
#include "core/systems.h"
#include "core/components.h"
#include <nlohmann/json.hpp>

tri::Application::Application()
    : camera_(glm::dvec3(0.0, 0.0, 5.0), glm::dvec3(0.0, -1.0, 0.0), -90.0f, 0.0f),
    updating_entities_(false)
{
    registry_ = std::make_unique<Registry>();
    ui_manager_ = std::make_unique<UiManager>();
}

void tri::Application::LoadPly(const std::string& path)
{
    if (splat_system_) {
        auto device = VulkanContext::Get().GetVkDevice();
        vkDeviceWaitIdle(device);
        splat_system_.reset();
    }

    auto* root_hierarchy = registry_->GetComponent<HierarchyComponent>(root_entity_);
    if (root_hierarchy) {
        std::vector<Entity> children_to_destroy = root_hierarchy->children;
        root_hierarchy->children.clear();
        for (auto child : children_to_destroy) {
            registry_->Destroy(child);
        }
    }

    Entity splat_id = registry_->Create();
    registry_->AddComponent<TransformComponent>(splat_id);
    registry_->AddComponent<HierarchyComponent>(splat_id);
    
    // Component pointers may be invalidated by Destroy(), so re-fetch root_hierarchy
    root_hierarchy = registry_->GetComponent<HierarchyComponent>(root_entity_);
    if (root_hierarchy) {
        root_hierarchy->children.push_back(splat_id);
    }
    registry_->GetComponent<HierarchyComponent>(splat_id)->parent = root_entity_;

    splat_system_ = std::make_unique<SplatSystem>(path, renderer_.get());
    splat_system_->Initialize(*registry_, splat_id, renderer_.get());
}

tri::Application::~Application()
{
}

void tri::Application::OnInitialize()
{
    renderer_ = std::make_unique<Renderer>(this);
    auto extent = VulkanContext::Get().GetSwapchain()->GetExtent();
    renderer_->Initialize((float)extent.width, (float)extent.height);

    root_entity_ = registry_->Create();
    registry_->AddComponent<TransformComponent>(root_entity_);
    registry_->AddComponent<HierarchyComponent>(root_entity_);
}

void tri::Application::OnCleanup()
{
    // Vulkan resources must be released before the device is destroyed.
    splat_system_.reset();
    registry_.reset();

    if (renderer_)
    {
        renderer_->Shutdown();
        renderer_.reset();
    }
}

void tri::Application::OnDrawFrame()
{
    static auto last_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float, std::chrono::seconds::period> diff = current_time - last_time;
    float delta_time = diff.count();
    last_time = current_time;

    float fps = (delta_time > 0.0f) ? (1.0f / delta_time) : 0.0f;
    ui_manager_->SetFps(fps);

    // Update Renderer matrices
    renderer_->SetViewMatrix(camera_.GetViewMatrix());
    renderer_->SetProjectionMatrix(camera_.GetProjectionMatrix(width_ / height_));
    renderer_->SetCameraPosition(camera_.GetPosition());
    renderer_->UpdateUniformBuffer();

    TransformSystem::Update(*registry_);
    renderer_->UpdateTransformBuffer(*registry_);

    if (root_entity_ != NullEntity)
    {
        // Use member splat_system_ for specific logic (like sorting)
        if (splat_system_)
        {
            registry_->ForEach<SplatDataComponent>([&](Entity entity, SplatDataComponent& data) {
                splat_system_->UpdateWithCamera(*registry_, entity, camera_);
            });
        }
    }

    renderer_->Draw(root_entity_);
}

void tri::Application::ProcessInput(const Uint8* state, float delta_time)
{
    camera_.ProcessKeyboard(state, delta_time);
}

void tri::Application::ProcessMouseMotion(float xrel, float yrel)
{
    camera_.ProcessMouseMovement(xrel, yrel);
}

void tri::Application::ProcessMouseScroll(float yoffset)
{
    camera_.ProcessMouseScroll(yoffset);
}

void tri::Application::ProcessMousePanning(float xrel, float yrel)
{
    camera_.ProcessMousePanning(xrel, yrel);
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

int tri::Application::Run(const std::string& json_args)
{
    std::string app_title = "Trinity";
    try
    {
        auto args = nlohmann::json::parse(json_args);
        if (args.contains("title"))
        {
            app_title = args["title"];
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Failed to parse arguments: " << e.what() << std::endl;
    }

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        std::cerr << "SDL_Init failed" << std::endl;
        return -1;
    }

    SDL_Window* window = nullptr;

    try
    {
        window = SDL_CreateWindow(app_title.c_str(),
                1280, 720,
                SDL_WINDOW_VULKAN | SDL_WINDOW_HIGH_PIXEL_DENSITY);

        if (!window)
        {
            throw std::runtime_error("SDL_CreateWindow failed");
        }

        Sdl3SurfaceProvider surface_provider(window);

        auto& vulkan_ctx = VulkanContext::Get();

        vulkan_ctx.GetWindowSystemExtensions = [=](auto& extension_list)
        {
            uint32_t ext_count = 0;
            char const* const* extensions =
                SDL_Vulkan_GetInstanceExtensions(&ext_count);
            if (ext_count > 0 && extensions != nullptr)
            {
                size_t current_size = extension_list.size();
                extension_list.resize(current_size + ext_count);
                for (uint32_t i = 0; i < ext_count; ++i)
                {
                    extension_list[current_size + i] = extensions[i];
                }
            }
        };

        vulkan_ctx.Initialize(app_title.c_str(), &surface_provider);
        vulkan_ctx.RecreateSwapchain();

        Application app;
        app.OnInitialize();

        app.GetUiManager().Initialize(window);
        app.GetUiManager().SetOnFileOpenCallback([&app](const std::string& path) {
            app.LoadPly(path);
        });

        // Set dimensions for projection matrix
        auto extent = vulkan_ctx.GetSwapchain()->GetExtent();
        app.width_ = (float)extent.width;
        app.height_ = (float)extent.height;

        bool is_running = true;
        while (is_running)
        {
            SDL_Event event;
            const Uint8* state = (const Uint8*)SDL_GetKeyboardState(nullptr);
            // Rough delta time for now
            app.ProcessInput(state, 0.016f);

            while (SDL_PollEvent(&event))
            {
                app.GetUiManager().ProcessEvent(&event);

                if (event.type == SDL_EVENT_QUIT)
                {
                    is_running = false;
                }
                else if (event.type == SDL_EVENT_MOUSE_MOTION)
                {
                    if (event.motion.state & SDL_BUTTON_LMASK)
                    {
                        app.ProcessMouseMotion(event.motion.xrel, event.motion.yrel);
                    }
                    else if (event.motion.state & SDL_BUTTON_MMASK)
                    {
                        app.ProcessMousePanning(event.motion.xrel, event.motion.yrel);
                    }
                }
                else if (event.type == SDL_EVENT_MOUSE_WHEEL)
                {
                    app.ProcessMouseScroll(event.wheel.y);
                }
            }

            app.GetUiManager().BeginFrame();
            app.OnDrawFrame();
        }
        // cleanup
        app.GetUiManager().Shutdown();
        app.OnCleanup();
        vulkan_ctx.Cleanup();

    }
    catch (const std::exception &e)
    {
        std::cerr << "Fatal Error: " << e.what() << std::endl;
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Fatal Error", e.what(), window);
    }

    if (window)
    {
        SDL_DestroyWindow(window);
    }
    SDL_Quit();

    return 0;
}
