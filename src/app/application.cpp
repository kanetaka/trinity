#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "app/application.h"
#include "renderer/renderer.h"
#include "scene/object.h"
#include "scene/component/splat_component.h"
#include "graphics/graphics_context.h"
#include "graphics/surface/sdl3_surface_provider.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <nlohmann/json.hpp>

using namespace tri;

Application::Application()
    : camera_(glm::dvec3(0.0, 0.0, 5.0), glm::dvec3(0.0, -1.0, 0.0), -90.0f, 0.0f)
{
    graphics_context_ = std::make_unique<GraphicsContext>();
    scene_ = std::make_unique<Scene>();
    ui_manager_ = std::make_unique<UiManager>();
}

void Application::LoadPly(const std::string& path)
{
    if (renderer_)
    {
        renderer_->WaitIdle();
    }

    Object& root = scene_->GetRoot();
    scene_->DestroyChildren(root);

    Object& splat_object = scene_->CreateObject(root, "Splat");
    auto& splat = splat_object.AddComponent<SplatComponent>(*graphics_context_, path);
    renderer_->RegisterComponent(splat);
}

Application::~Application()
{
}

void Application::OnInitialize()
{
    renderer_ = std::make_unique<Renderer>(*graphics_context_);
    renderer_->Initialize();
    width_ = renderer_->GetScreenWidth();
    height_ = renderer_->GetScreenHeight();
}

void Application::OnCleanup()
{
    // Vulkan resources must be released before the device is destroyed.
    scene_.reset();

    if (renderer_)
    {
        renderer_->Shutdown();
        renderer_.reset();
    }

    if (graphics_context_)
    {
        graphics_context_->Cleanup();
    }
}

void Application::OnDrawFrame()
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

    scene_->Update();
    renderer_->UpdateTransformBuffer(*scene_);

    for (auto* object : scene_->GetOrderedObjects())
    {
        if (auto* splat = object->GetComponent<SplatComponent>())
        {
            splat->UpdateWithCamera(camera_, object->GetWorldTransform());
        }
    }

    renderer_->Draw(scene_->GetRoot(), [this](auto& command_buffer)
        {
            ui_manager_->Render(command_buffer);
        });
}

void Application::ProcessInput(const Uint8* state, float delta_time)
{
    camera_.ProcessKeyboard(state, delta_time);
}

void Application::ProcessMouseMotion(float xrel, float yrel)
{
    camera_.ProcessMouseMovement(xrel, yrel);
}

void Application::ProcessMouseScroll(float yoffset)
{
    camera_.ProcessMouseScroll(yoffset);
}

void Application::ProcessMousePanning(float xrel, float yrel)
{
    camera_.ProcessMousePanning(xrel, yrel);
}

#if defined(__ANDROID__)
void Application::OnSurfaceChanged()
{
    if (graphics_context_)
    {
        graphics_context_->RecreateSwapchain();
    }
    if (renderer_)
    {
        renderer_->Initialize();
        width_ = renderer_->GetScreenWidth();
        height_ = renderer_->GetScreenHeight();
    }
}
#endif

int Application::Run(const std::string& json_args)
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
        std::cerr << "JSON Parse error: " << e.what() << std::endl;
    }

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS))
    {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
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

        Application app;
        app.GetGraphicsContext().Initialize(app_title.c_str(), &surface_provider);
        app.GetGraphicsContext().RecreateSwapchain();

        app.OnInitialize();

        app.GetUiManager().Initialize(window, app.GetGraphicsContext());
        app.GetUiManager().SetOnFileOpenCallback([&app](const std::string& path) {
            app.LoadPly(path);
        });

        // Dimensions are already set in OnInitialize via Renderer
        app.width_ = app.GetRenderer()->GetScreenWidth();
        app.height_ = app.GetRenderer()->GetScreenHeight();

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
