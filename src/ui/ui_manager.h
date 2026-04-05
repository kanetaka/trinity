#pragma once
#include <functional>
#include <memory>
#include <string>

union SDL_Event;
struct SDL_Window;

namespace tri
{
    class CommandBuffer;

    class UiManager
    {
    public:
        UiManager();
        ~UiManager();

        void Initialize(SDL_Window* window);
        void Shutdown();

        void BeginFrame();
        void Render(std::shared_ptr<CommandBuffer>& command_buffer);

        bool ProcessEvent(const SDL_Event* event);

        void SetOnFileOpenCallback(std::function<void(const std::string&)> callback)
        {
            on_file_open_ = callback;
        }

    private:
        void ShowMenu();
        void OpenFileDialog();

        std::function<void(const std::string&)> on_file_open_;
        bool is_initialized_{false};
    };
}
