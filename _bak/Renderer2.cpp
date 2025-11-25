#include "renderer.h"

using namespace x8;

Renderer::Renderer() {
	SDL_Init(SDL_INIT_EVERYTHING);

	window_ = SDL_CreateWindow(window_name_.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		800, 600,
		SDL_WINDOW_VULKAN | SDL_WINDOW_SHOWN);

	InitVulkan();
}

Renderer::~Renderer() {
    delete vulkan_;
    vulkan_ = nullptr;

    SDL_DestroyWindow(window_);
    window_ = nullptr;

    SDL_Quit();
}

void Renderer::InitVulkan() {
    vulkan_ = new Vulkan();
    init_vulkan_extern();
}

void Renderer::Run() {
	SDL_Event event;
	bool running = true;

	while (running) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				running = false;
			}
		}

		AcquireNextImage();
		ResetCommandBuffer();
		BeginCommandBuffer();
		{
			VkClearColorValue clear_color = { 0.0f, 0.0f, 0.8f, 1.0f };
			VkClearDepthStencilValue clear_depth_stencil = { 1.0f, 0 };
			BeginRenderPass(clear_color, clear_depth_stencil); {
			}
			EndRenderPass();
		}
		EndCommandBuffer();

		QueueSubmit();
		QueuePresent();
	}
}

void Renderer::init_vulkan_extern() {
    ////////////////////////////////////////////////////
    ///////             [Core]
    //////////////////////////////////////////
    vulkan_->Create_Instance();
    vulkan_->Create_Debug();
    vulkan_->Create_Surface();
    vulkan_->Select_PhysicalDevice();
    vulkan_->Select_QueueFamily();
    vulkan_->Create_Device();

    ////////////////////////////////////////////////////
    ///////           [Screen]
    //////////////////////////////////////////
    bool test = vulkan_->Create_Swapchain(false);
    vulkan_->Create_ImageViews();
    vulkan_->Setup_DepthStencil();
    vulkan_->Create_RenderPass();
    vulkan_->Create_Framebuffers();

    ///////////////////////////////////////////////////////////

    vulkan_->createCommandPool();
    vulkan_->createCommandBuffers();
    vulkan_->create_semaphores();
    vulkan_->createFences();
}
