#pragma once
#include <functional>
#include <memory>
#include <string>

union SDL_Event;
struct SDL_Window;

namespace tri
{
    class CommandBuffer;
    class GraphicsContext;

    class UiManager
    {
    public:
        UiManager();
        ~UiManager();

        void Initialize(SDL_Window* window, GraphicsContext& graphics_ctx);
        void Shutdown();

        void BeginFrame();
        void Render(CommandBuffer& command_buffer);

        bool ProcessEvent(const SDL_Event* event);

        void SetOnFileOpenCallback(std::function<void(const std::string&)> callback)
        {
            on_file_open_ = callback;
        }

        void SetOnOpenPointCloudCallback(std::function<void(const std::string&)> callback)
        {
            on_open_point_cloud_ = callback;
        }

        void SetOnOpen3dgsCallback(std::function<void(const std::string&)> callback)
        {
            on_open_3dgs_ = callback;
        }

        void SetPointSizePtr(float* point_size_ptr)
        {
            point_size_ptr_ = point_size_ptr;
        }

        void SetFps(float fps) { fps_ = fps; }

    private:
        void ShowMenu();
        void ShowPointCloudSettings();
        void OpenFileDialog(const std::function<void(const std::string&)>& callback);

        std::function<void(const std::string&)> on_file_open_;
        std::function<void(const std::string&)> on_open_point_cloud_;
        std::function<void(const std::string&)> on_open_3dgs_;
        float* point_size_ptr_{ nullptr };
        bool is_initialized_{ false };
        bool show_fps_{ true };
        bool show_point_cloud_settings_{ true };
        float fps_{ 0.0f };
    };
}


