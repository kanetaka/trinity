#include "swapchain.h"
#include <stdexcept>
#include <cassert>

bool Swapchain::Recreate(uint32_t width, uint32_t height)
{
        auto& vulkan_ctx = VulkanContext::Get();
        auto vk_physical_device = vulkan_ctx.GetVkPhysicalDevice();
        auto vk_device = vulkan_ctx.GetVkDevice();
        auto surface = vulkan_ctx.GetSurface();

        VkSurfaceCapabilitiesKHR caps;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device, surface, &caps);
        VkExtent2D extent = caps.currentExtent;
        if (extent.width == UINT32_MAX)
        {
                extent.width = width;
                extent.height = height;
        }

        uint32_t count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, surface, &count, nullptr);
        std::vector<VkSurfaceFormatKHR> formats(count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device, surface, &count, formats.data());

        // 出力フォーマットの選択
        VkSurfaceFormatKHR format = formats[0];
        for (auto& surface_format : formats)
        {
                if (surface_format.colorSpace != VK_COLORSPACE_SRGB_NONLINEAR_KHR)
                {
                        continue;
                }

                if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM ||
                        surface_format.format == VK_FORMAT_R8G8B8A8_UNORM)
                {
                        format = surface_format;
                        break;
                }
        }
        auto image_count = std::max(3u, caps.minImageCount);

        VkSwapchainCreateInfoKHR info{};
        info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        info.surface = surface;
        info.minImageCount = image_count;
        info.imageFormat = format.format;
        info.imageColorSpace = format.colorSpace;
        info.imageExtent = extent;
        info.imageArrayLayers = 1;
        info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
        info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
        info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
        info.clipped = VK_TRUE;
        info.oldSwapchain = swapchain_;

        // GPUがアイドル状態になってからスワップチェインの(再)作成
        vkDeviceWaitIdle(vk_device);

        VkSwapchainKHR swapchain{};
        if (auto res = vkCreateSwapchainKHR(vk_device, &info, nullptr, &swapchain); res != VK_SUCCESS)
                throw std::runtime_error("failed to create swapchain");
        if (swapchain_ != VK_NULL_HANDLE)
        {
                // 古いものを破棄する
                vkDestroySwapchainKHR(vk_device, swapchain_, nullptr);
                swapchain_ = VK_NULL_HANDLE;

                for (auto& view : image_views_)
                {
                        vkDestroyImageView(vk_device, view, nullptr);
                }
                image_views_.clear();

                DestroyFrameContext();
        }

        swapchain_ = swapchain;
        image_format_ = format;
        image_extent_ = extent;

        vkGetSwapchainImagesKHR(vk_device, swapchain_, &image_count, nullptr);
        images_.resize(image_count);
        vkGetSwapchainImagesKHR(vk_device, swapchain_, &image_count, images_.data());

        for (uint32_t i = 0; i < images_.size(); ++i)
        {
                VkImageViewCreateInfo image_view_ci{
                        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
                        .image = images_[i],
                        .viewType = VK_IMAGE_VIEW_TYPE_2D,
                        .format = format.format,
                        .components = {
                                VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY,
                                VK_COMPONENT_SWIZZLE_IDENTITY,
                        },
                        .subresourceRange = {
                                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                .baseMipLevel = 0,
                                .levelCount = 1,
                                .baseArrayLayer = 0,
                                .layerCount = 1,
                        }
                };
                VkImageView view;
                vkCreateImageView(vk_device, &image_view_ci, nullptr, &view);
                image_views_.push_back(view);
        }

        CreateFrameContext();
        return true;
}

void Swapchain::Cleanup()
{
        auto& vulkan_ctx = VulkanContext::Get();
        auto vk_device = vulkan_ctx.GetVkDevice();
        DestroyFrameContext();

        for (auto& view : image_views_)
        {
                vkDestroyImageView(vk_device, view, nullptr);
        }
        if (swapchain_)
        {
                vkDestroySwapchainKHR(vk_device, swapchain_, nullptr);
                swapchain_ = VK_NULL_HANDLE;
        }
        images_.clear();
        image_views_.clear();
}

VkResult Swapchain::AcquireNextImage()
{
        auto& vulkan_ctx = VulkanContext::Get();
        auto vk_device = vulkan_ctx.GetVkDevice();

        // プレゼンテーション完了待ちで使用するセマフォの取得
        assert(!present_semaphore_list_.empty());
        VkSemaphore acquire_semaphore = present_semaphore_list_.back();
        present_semaphore_list_.pop_back();

        auto result = vkAcquireNextImageKHR(
            vk_device, swapchain_, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &current_index_);
        if (result != VK_SUCCESS)
        {
            present_semaphore_list_.push_back(acquire_semaphore);
            return result;
        }

        VkSemaphore old_semaphore = frames_[current_index_].presentComplete;
        if (old_semaphore != VK_NULL_HANDLE)
        {
            present_semaphore_list_.push_back(old_semaphore);
        }
        frames_[current_index_].presentComplete = acquire_semaphore;

        return result;
}

VkResult Swapchain::QueuePresent(VkQueue queuePresent)
{
        VkPresentInfoKHR present_info{};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swapchain_;
        present_info.pImageIndices = &current_index_;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &frames_[current_index_].renderComplete;

        auto& vulkan_ctx = VulkanContext::Get();
        auto result = vkQueuePresentKHR(queuePresent, &present_info);

        return result;
}

VkSemaphore Swapchain::GetPresentCompleteSemaphore() const
{
    return frames_[current_index_].presentComplete;
}

VkSemaphore Swapchain::GetRenderCompleteSemaphore() const
{
    return frames_[current_index_].renderComplete;
}

void Swapchain::CreateFrameContext()
{
    auto& vulkan_ctx = VulkanContext::Get();
    auto vk_device = vulkan_ctx.GetVkDevice();
    frames_.resize(images_.size());
    uint32_t index = 0;
    for (auto& frame : frames_)
    {
        VkSemaphoreCreateInfo sem_ci{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        vkCreateSemaphore(vk_device, &sem_ci, nullptr, &frame.renderComplete);
    }

    uint32_t present_complete_semaphore_count = images_.size() + 1;
    present_semaphore_list_.reserve(present_complete_semaphore_count);
    for (uint32_t i = 0; i < present_complete_semaphore_count; ++i)
    {
        VkSemaphoreCreateInfo semaphore_ci{
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        };
        VkSemaphore semaphore;
        vkCreateSemaphore(vk_device, &semaphore_ci, nullptr, &semaphore);
        present_semaphore_list_.push_back(semaphore);
    }
}

void Swapchain::DestroyFrameContext()
{
    auto& vulkan_ctx = VulkanContext::Get();
    auto vk_device = vulkan_ctx.GetVkDevice();
    for (auto& frame : frames_)
    {
        vkDestroySemaphore(vk_device, frame.presentComplete, nullptr);
        vkDestroySemaphore(vk_device, frame.renderComplete, nullptr);
    }
    frames_.clear();
    for (auto& sem : present_semaphore_list_)
    {
        vkDestroySemaphore(vk_device, sem, nullptr);
    }
    present_semaphore_list_.clear();
}
