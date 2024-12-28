#pragma once

#include <vulkan/vulkan.h>
#include <iostream>
#include <vector>
using namespace std;

class Vulkan {
private:
    void CreateSemaphore(VkSemaphore *semaphore);
public:
    Vulkan();
    ~Vulkan();

    // global CreateImageView
    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
    // global CreateImage
    void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    // global FindMemoryType
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    // **** [Core] **** //
    void createInstance();
    void createDebug();
    void createSurface();
    void selectPhysicalDevice();
    void selectQueueFamily();
    void createDevice();

    // ****  [Screen] **** //
    bool createSwapchain(bool resize);
    void createImageViews();
    void setupDepthStencil();
    void createRenderPass();
    void createFramebuffers();

    /////////////////////////
    void createCommandPool();
    void createCommandBuffers();
    void createSemaphores();
    void createFences();

    VkInstance instance_;
    vector<VkExtensionProperties> instance_extension_;
    VkDebugReportCallbackEXT debug_callback_;
    VkSurfaceKHR surface_;
    VkPhysicalDevice physical_devices_;
    uint32_t graphics_queue_family_index_;
    uint32_t present_queue_family_index_;
    VkDevice device_;
    VkQueue graphics_queue_;
    VkQueue present_queue_;


    VkSwapchainKHR swapchain_;
    VkSurfaceCapabilitiesKHR surface_capabilities_;
    VkSurfaceFormatKHR surface_format_;
    VkExtent2D swapchain_size_;
    vector<VkImage> swapchain_images_;
    uint32_t swapchain_image_count_;

    vector<VkImageView> swapchain_image_views_;
    VkFormat depth_format_;
    VkImage depth_image_;
    VkDeviceMemory depth_image_memory_;
    VkImageView depth_image_view_;

    VkRenderPass render_pass_;

    vector<VkFramebuffer> swapchain_framebuffers_;

    VkCommandPool command_pool_;
    vector<VkCommandBuffer> command_buffers_;
    VkSemaphore image_available_semaphore_;
    VkSemaphore rendering_finished_semaphore_;
    vector<VkFence> fences_;
};

void InitVulkanExtern(Vulkan *vulkan);
