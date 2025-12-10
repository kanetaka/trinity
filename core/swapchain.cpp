#include "core/swapchain.h"
#include <stdexcept>
#include <cassert>


// ------------------------------- //
// ---- Public Member Methods ---- //
// ------------------------------- //

bool Swapchain::Recreate(uint32_t width, uint32_t height) {
    auto& vulkan_context = VulkanContext::Get();
    auto physical_device = vulkan_context.GetPhysicalDevice();
    auto device = vulkan_context.GetDevice();
    auto surface = vulkan_context.GetSurface();

    VkSurfaceCapabilitiesKHR caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &caps);
    VkExtent2D extent = caps.currentExtent;
    if (extent.height == UINT32_MAX) {
        extent.width = width;
        extent.height = height;
    }

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(format_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());

    // Choose surface format
    VkSurfaceFormatKHR format = formats[0];
    for (auto& surface_format : formats) {
        if (surface_format.colorSpace != VK_COLORSPACE_SRGB_NONLINEAR_KHR) {
            continue;
        }
        if (surface_format.format == VK_FORMAT_B8G8R8A8_UNORM ||
            surface_format.format == VK_FORMAT_R8G8B8A8_UNORM) {
            format = surface_format;
            break;
        }
    }
    auto image_count = std::max(3u, caps.minImageCount);

    VkSwapchainCreateInfoKHR swapchain_info {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .flags = 0,
        .surface = surface,
        .minImageCount = image_count,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .queueFamilyIndexCount = 0,
        .pQueueFamilyIndices = nullptr,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = VK_TRUE,
        .oldSwapchain = swapchain_,
    };

    vkDeviceWaitIdle(device);

    VkSwapchainKHR swapchain{};
    if (vkCreateSwapchainKHR(device, &swapchain_info, nullptr, &swapchain) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create swapchain");
    }

    swapchain_ = swapchain;
    image_format_ = format;
    image_extent_ = extent;

    vkGetSwapchainImagesKHR(device, swapchain_, &image_count, nullptr);
    images_.resize(image_count);
    vkGetSwapchainImagesKHR(device, swapchain_, &image_count, images_.data());

    for (uint32_t i = 0; i < images_.size(); ++i) {
        VkImageViewCreateInfo image_view_info {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .image = images_[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = image_format_.format,
            .components = {
                .r = VK_COMPONENT_SWIZZLE_IDENTITY,
                .g = VK_COMPONENT_SWIZZLE_IDENTITY,
                .b = VK_COMPONENT_SWIZZLE_IDENTITY,
                .a = VK_COMPONENT_SWIZZLE_IDENTITY,
            },
            .subresourceRange = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        };
        VkImageView image_view;
        vkCreateImageView(device, &image_view_info, nullptr, &image_view);
        image_views_.push_back(image_view);
    }
    CreateFrameContext();

    return true;
}

void Swapchain::Cleanup() {
    auto& vulkan_context = VulkanContext::Get();
    auto device = vulkan_context.GetDevice();
    for (auto& view : image_views_) {
        vkDestroyImageView(device, view, nullptr);
    }

    if (swapchain_) {
        vkDestroySwapchainKHR(device, swapchain_, nullptr);
        swapchain_ = VK_NULL_HANDLE;
    }

    images_.clear();
    image_views_.clear();
}

VkResult Swapchain::AcquireNextImage() {
    auto& vulkan_context = VulkanContext::Get();
    auto device = vulkan_context.GetDevice();

    assert(!present_semaphores_.empty());
    VkSemaphore acquire_semaphore = present_semaphores_.back();
    present_semaphores_.pop_back();

    auto result = vkAcquireNextImageKHR(device, swapchain_, UINT64_MAX, acquire_semaphore, VK_NULL_HANDLE, &current_index_);
    if (result != VK_SUCCESS) {
        present_semaphores_.push_back(acquire_semaphore);
        return result;
    }

    VkSemaphore old_semaphore = frames_[current_index_].present_complete;
    if (old_semaphore != VK_NULL_HANDLE) {
        present_semaphores_.push_back(old_semaphore);
    }
    frames_[current_index_].present_complete = acquire_semaphore;

    return result;
}

VkResult Swapchain::QueuePresent(VkQueue queue_present) {
    VkPresentInfoKHR present_info {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frames_[current_index_].render_complete,
        .swapchainCount = 1,
        .pSwapchains = &swapchain_,
        .pImageIndices = &current_index_,
        .pResults = nullptr,
    };

    auto& vulkan_context = VulkanContext::Get();
    auto result = vkQueuePresentKHR(queue_present, &present_info);

    return result;
}

VkSemaphore Swapchain::GetRenderCompleteSemaphore() const {
    return frames_[current_index_].render_complete;
}

VkSemaphore Swapchain::GetPresentCompleteSemaphore() const {
    return frames_[current_index_].present_complete;
}


// -------------------------------- //
// ---- Private Member Methods ---- //
// -------------------------------- //

void Swapchain::CreateFrameContext() {
    auto& vulkan_context = VulkanContext::Get();
    auto device = vulkan_context.GetDevice();

    frames_.resize(images_.size());

    for (auto& frame : frames_) {
        VkSemaphoreCreateInfo semaphore_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };
        if (vkCreateSemaphore(device, &semaphore_info, nullptr, &frame.render_complete) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create render complete semaphore");
        }
    }

    uint32_t present_complete_semaphore_count = images_.size() + 1;
    present_semaphores_.reserve(present_complete_semaphore_count);

    for (uint32_t i = 0; i < present_complete_semaphore_count; ++i) {
        VkSemaphoreCreateInfo semaphore_info {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
        };
        VkSemaphore semaphore;
        vkCreateSemaphore(device, &semaphore_info, nullptr, &semaphore);
        present_semaphores_.push_back(semaphore);
    }
}

void Swapchain::DestroyFrameContext() {
    auto& vulkan_context = VulkanContext::Get();
    auto device = vulkan_context.GetDevice();

    for (auto& frame : frames_) {
        vkDestroySemaphore(device, frame.present_complete, nullptr);
        vkDestroySemaphore(device, frame.render_complete, nullptr);
    }

    frames_.clear();
    for (auto semaphore : present_semaphores_) {
        vkDestroySemaphore(device, semaphore, nullptr);
    }

    present_semaphores_.clear();
}
