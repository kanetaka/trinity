#pragma once
#include "core/vulkan_context.h"
#include "core/surface_provider.h"
#include <map>
#include <string>

class VulkanContext;
class Swapchain
{
public:
        Swapchain() = default;

        bool Recreate(uint32_t newWidth, uint32_t newHeight);
        void Cleanup();

        VkResult AcquireNextImage();
        VkResult QueuePresent(VkQueue queuePresent);

        operator const VkSwapchainKHR()       { return swapchain_; }

        VkSurfaceFormatKHR GetFormat() const    { return image_format_; }
        VkExtent2D  GetExtent() const           { return image_extent_; }

        uint32_t GetCurrentIndex() const { return current_index_; }
        uint32_t GetImageCount() const { return uint32_t(images_.size()); }
        VkImage  GetCurrentImage() const { return images_[current_index_]; }
        VkImageView  GetCurrentView() const { return image_views_[current_index_]; }

        VkSemaphore GetPresentCompleteSemaphore() const;
        VkSemaphore GetRenderCompleteSemaphore() const;
        std::vector<VkImageView> GetImageViews() const { return image_views_; }
private:
        void CreateFrameContext();
        void DestroyFrameContext();

        VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
        uint32_t current_index_ = 0;

        VkSurfaceFormatKHR image_format_{};
        VkExtent2D image_extent_{};
        std::vector<VkImage> images_;
        std::vector<VkImageView> image_views_;

        struct FrameContext
        {
            VkSemaphore renderComplete = VK_NULL_HANDLE;
            VkSemaphore presentComplete = VK_NULL_HANDLE;
        };
        std::vector<FrameContext> frames_;
        std::vector<VkSemaphore> present_semaphore_list_;
        friend class VulkanContext;
};
