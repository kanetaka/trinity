#pragma once
#include "core/vulkan_context.h"
#include "core/surface_provider.h"

class VulkanContext;

class Swapchain {
public:
    Swapchain() = default;

public:
    bool Recreate(uint32_t new_width, uint32_t new_height);
    void Cleanup();

    VkResult AcquireNextImage();
    VkResult QueuePresent(VkQueue queue_present);

    operator const VkSwapchainKHR() { return swapchain_; }

	VkSurfaceFormatKHR GetImageFormat() const { return image_format_; }
	VkExtent2D GetImageExtent() const { return image_extent_; }

	uint32_t GetCurrentIndex() const { return current_index_; }
	uint32_t GetImageCount() const { return static_cast<uint32_t>(images_.size()); }
	VkImage GetCurrentImage() const { return images_[current_index_]; }
	VkImageView GetCurrentImageView() const { return image_views_[current_index_]; }

    VkSemaphore GetRenderCompleteSemaphore() const;
    VkSemaphore GetPresentCompleteSemaphore() const;
	std::vector<VkImageView> GetImageViews() const { return image_views_; }

private:
    void CreateFrameContext();
    void DestroyFrameContext();

private:
    VkSwapchainKHR swapchain_{ VK_NULL_HANDLE };
	uint32_t current_index_{ 0 };

	VkSurfaceFormatKHR image_format_{};
    VkExtent2D image_extent_{};
    std::vector<VkImage> images_{};
    std::vector<VkImageView> image_views_{};

    struct FrameContext {
		VkSemaphore render_complete{ VK_NULL_HANDLE };
		VkSemaphore present_complete{ VK_NULL_HANDLE };
    };
    std::vector<FrameContext> frames_{};
	std::vector<VkSemaphore> present_semaphores_{};

    friend class VulkanContext;
};
