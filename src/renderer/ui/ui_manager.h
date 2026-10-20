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
    class ColmapReconstructor;

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

        void SetColmapReconstructor(ColmapReconstructor* reconstructor)
        {
            colmap_reconstructor_ = reconstructor;
        }

        void SetOnReconstructColmapCallback(std::function<void(const std::string&, int)> callback)
        {
            on_reconstruct_colmap_ = callback;
        }

        void SetPointSizePtr(float* point_size_ptr)
        {
            point_size_ptr_ = point_size_ptr;
        }

        void SetFps(float fps) { fps_ = fps; }

    private:
        void ShowMenu();
        void ShowPointCloudSettings();
        void ShowColmapDialog();
        void ShowOutputWindow();
        void OpenFileDialog(const std::function<void(const std::string&)>& callback);
        void OpenFolderDialog(const std::function<void(const std::string&)>& callback);

        std::function<void(const std::string&)> on_file_open_;
        std::function<void(const std::string&)> on_open_point_cloud_;
        std::function<void(const std::string&)> on_open_3dgs_;
        std::function<void(const std::string&, int)> on_reconstruct_colmap_;
        ColmapReconstructor* colmap_reconstructor_{ nullptr };
        bool show_colmap_dialog_{ false };
        std::string colmap_selected_folder_;
        int colmap_quality_index_{ 1 };
        float* point_size_ptr_{ nullptr };
        bool is_initialized_{ false };
        bool show_fps_{ true };
        bool show_point_cloud_settings_{ true };
        bool show_output_window_{ true };
        bool output_auto_scroll_{ true };
        bool output_show_info_{ true };
        bool output_show_warn_{ true };
        bool output_show_error_{ true };
        char output_filter_[128]{ 0 };
        float fps_{ 0.0f };
    };
}


