#include "ui/ui_manager.h"
#include "render/vulkan_context.h"
#include "render/swapchain.h"
#include "render/command_buffer.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_vulkan.h"

#if defined(_WIN32)
#include <windows.h>
#include <commdlg.h>
#include <codecvt>
#include <locale>
#endif

namespace tri
{
    UiManager::UiManager() {}

    UiManager::~UiManager()
    {
        Shutdown();
    }

    void UiManager::Initialize(SDL_Window* window)
    {
        if (is_initialized_) return;

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        
        ImGui::StyleColorsDark();

        auto& vulkan_ctx = VulkanContext::Get();

        ImGui_ImplSDL3_InitForVulkan(window);

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = vulkan_ctx.GetVkInstance();
        init_info.PhysicalDevice = vulkan_ctx.GetVkPhysicalDevice();
        init_info.Device = vulkan_ctx.GetVkDevice();
        init_info.QueueFamily = vulkan_ctx.GetGraphicsFamily();
        init_info.Queue = vulkan_ctx.GetGraphicsQueue();
        init_info.PipelineCache = VK_NULL_HANDLE;
        init_info.DescriptorPoolSize = 1000;
        init_info.MinImageCount = 2;
        init_info.ImageCount = vulkan_ctx.GetSwapchain()->GetImageCount();
        init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        
        init_info.UseDynamicRendering = true;
        VkFormat color_format = vulkan_ctx.GetSwapchain()->GetFormat().format;
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
        init_info.PipelineInfoMain.PipelineRenderingCreateInfo.pColorAttachmentFormats = &color_format;

        ImGui_ImplVulkan_Init(&init_info);

        is_initialized_ = true;
    }

    void UiManager::Shutdown()
    {
        if (!is_initialized_) return;

        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

        is_initialized_ = false;
    }

    void UiManager::BeginFrame()
    {
        if (!is_initialized_) return;

        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        ShowMenu();
    }

    bool UiManager::ProcessEvent(const SDL_Event* event)
    {
        if (!is_initialized_) return false;
        return ImGui_ImplSDL3_ProcessEvent(event);
    }

    void UiManager::Render(std::shared_ptr<CommandBuffer>& command_buffer)
    {
        if (!is_initialized_) return;

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        if (draw_data)
        {
            ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer->Get());
        }
    }

    void UiManager::ShowMenu()
    {
        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Import 3DGS(*.ply)"))
                {
                    OpenFileDialog();
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }
    }

    void UiManager::OpenFileDialog()
    {
#if defined(_WIN32)
        wchar_t szFile[260] = { 0 };
        OPENFILENAMEW ofn = { 0 };
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = nullptr;  // Application window handle can be passed here if needed
        ofn.lpstrFile = szFile;
        ofn.nMaxFile = sizeof(szFile) / sizeof(wchar_t);
        ofn.lpstrFilter = L"PLY Files\0*.ply\0All Files\0*.*\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = nullptr;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = nullptr;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

        if (GetOpenFileNameW(&ofn) == TRUE)
        {
#pragma warning(push)
#pragma warning(disable: 4996)
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::string open_path = converter.to_bytes(ofn.lpstrFile);
#pragma warning(pop)
            if (on_file_open_)
            {
                on_file_open_(open_path);
            }
        }
#else
        // TODO: Other Platforms file dialog
#endif
    }
}
