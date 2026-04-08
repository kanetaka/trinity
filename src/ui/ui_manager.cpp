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
        
        // Set Dark Red Theme
        auto& style = ImGui::GetStyle();
        auto* colors = style.Colors;

        colors[ImGuiCol_Text]                   = ImVec4(0.95f, 0.96f, 0.98f, 1.00f);
        colors[ImGuiCol_TextDisabled]           = ImVec4(0.36f, 0.42f, 0.47f, 1.00f);
        colors[ImGuiCol_WindowBg]               = ImVec4(0.12f, 0.04f, 0.04f, 0.94f);
        colors[ImGuiCol_ChildBg]                = ImVec4(0.15f, 0.05f, 0.05f, 0.00f);
        colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.03f, 0.03f, 0.94f);
        colors[ImGuiCol_Border]                 = ImVec4(0.40f, 0.10f, 0.10f, 0.50f);
        colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]                = ImVec4(0.20f, 0.05f, 0.05f, 0.54f);
        colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.40f, 0.10f, 0.10f, 0.40f);
        colors[ImGuiCol_FrameBgActive]          = ImVec4(0.50f, 0.12f, 0.12f, 0.67f);
        colors[ImGuiCol_TitleBg]                = ImVec4(0.15f, 0.05f, 0.05f, 1.00f);
        colors[ImGuiCol_TitleBgActive]          = ImVec4(0.40f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg]              = ImVec4(0.60f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.02f, 0.01f, 0.01f, 0.53f);
        colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.31f, 0.08f, 0.08f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.41f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.51f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_CheckMark]              = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_SliderGrab]             = ImVec4(0.70f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.90f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_Button]                 = ImVec4(0.40f, 0.08f, 0.08f, 0.40f);
        colors[ImGuiCol_ButtonHovered]          = ImVec4(0.60f, 0.12f, 0.12f, 1.00f);
        colors[ImGuiCol_ButtonActive]           = ImVec4(0.75f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_Header]                 = ImVec4(0.40f, 0.08f, 0.08f, 0.31f);
        colors[ImGuiCol_HeaderHovered]          = ImVec4(0.85f, 0.20f, 0.20f, 1.00f);
        colors[ImGuiCol_HeaderActive]           = ImVec4(0.95f, 0.25f, 0.25f, 1.00f);
        colors[ImGuiCol_Separator]              = ImVec4(0.43f, 0.10f, 0.10f, 0.50f);
        colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.75f, 0.15f, 0.15f, 0.78f);
        colors[ImGuiCol_SeparatorActive]        = ImVec4(0.75f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_ResizeGrip]             = ImVec4(0.40f, 0.08f, 0.08f, 0.20f);
        colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.60f, 0.12f, 0.12f, 0.67f);
        colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.75f, 0.15f, 0.15f, 0.95f);
        colors[ImGuiCol_Tab]                    = ImVec4(0.25f, 0.06f, 0.06f, 0.86f);
        colors[ImGuiCol_TabHovered]             = ImVec4(0.60f, 0.12f, 0.12f, 0.80f);
        colors[ImGuiCol_TabActive]              = ImVec4(0.50f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_TabUnfocused]           = ImVec4(0.12f, 0.04f, 0.04f, 0.97f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.20f, 0.06f, 0.06f, 1.00f);
        colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.75f, 0.15f, 0.15f, 0.35f);
        colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight]           = ImVec4(0.75f, 0.15f, 0.15f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);

        style.WindowRounding    = 6.0f;
        style.ChildRounding     = 4.0f;
        style.FrameRounding     = 3.0f;
        style.PopupRounding     = 4.0f;
        style.ScrollbarRounding = 9.0f;
        style.GrabRounding      = 3.0f;
        style.TabRounding       = 4.0f;
        style.WindowBorderSize  = 1.0f;
        style.WindowPadding     = ImVec2(10, 10);
        style.FramePadding      = ImVec2(5, 5);
        style.ItemSpacing       = ImVec2(10, 8);


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
