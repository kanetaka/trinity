#include "vulkan_extern.h"
#include <string>
#include <SDL2/SDL.h>
#include <SDL2/SDL_Vulkan.h>

#define CLAMP(x, lo, hi)    ((x) < (lo) ? (lo) : (x) > (hi) ? (hi) : (x))

void initVulkanExtern(Vulkan *vulkan) {
    // [Core]
    vulkan->createInstance();
    vulkan->createDebug();
    vulkan->createSurface();
    vulkan->selectPhysicalDevice();
    vulkan->selectQueueFamily();
    vulkan->createDevice();

    // [Screen]
    bool test = vulkan->createSwapchain(false);
    vulkan->createImageViews();
    vulkan->setupDepthStencil();
    vulkan->createRenderPass();
    vulkan->createFramebuffers();

    ///////////////////////////////////////////////////////////

    vulkan->createCommandPool();
    vulkan->createCommandBuffers();
    vulkan->createSemaphores();
    vulkan->createFences();
}

Vulkan::Vulkan() {
}

Vulkan::~Vulkan() {
}

// [Core]
extern SDL_Window *window__;
extern std::string window_name__;

const vector<const char*> validation_layers = {
    ///has bug
    //"VK_LAYER_LUNARG_standard_validation"
};

void Vulkan::createInstance() {
    unsigned int extension_count = 0;
    SDL_Vulkan_GetInstanceExtensions(window__, &extension_count, nullptr);
    vector<const char *> extension_names(extension_count);
    SDL_Vulkan_GetInstanceExtensions(window__, &extension_count, extension_names.data());

    VkApplicationInfo app_info = {};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = window_name__.c_str();
    app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.pEngineName = "No Engine";
    app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo instance_create_info = {};
    instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_create_info.pApplicationInfo = &app_info;
    instance_create_info.enabledLayerCount = validation_layers.size();
    instance_create_info.ppEnabledLayerNames = validation_layers.data();
    instance_create_info.enabledExtensionCount = extension_names.size();
    instance_create_info.ppEnabledExtensionNames = extension_names.data();

    vkCreateInstance(&instance_create_info, nullptr, &instance_);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL VulkanReportFunc(
        VkDebugReportFlagsEXT flags,
        VkDebugReportObjectTypeEXT objType,
        uint64_t obj,
        size_t location,
        int32_t code,
        const char* layerPrefix,
        const char* msg,
        void* userData) {
    printf("VULKAN VALIDATION: %s\n", msg);
    return VK_FALSE;
}

PFN_vkCreateDebugReportCallbackEXT sdl2_vk_create_debug_report_callback_ext = nullptr;
void Vulkan::createDebug() {
    sdl2_vk_create_debug_report_callback_ext = (PFN_vkCreateDebugReportCallbackEXT)SDL_Vulkan_GetVkGetInstanceProcAddr();

    VkDebugReportCallbackCreateInfoEXT debug_callback_create_info = {};
    debug_callback_create_info.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_callback_create_info.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
    debug_callback_create_info.pfnCallback = VulkanReportFunc;

    sdl2_vk_create_debug_report_callback_ext(instance_, &debug_callback_create_info, 0, &debug_callback_);
}

void Vulkan::createSurface() {
    SDL_Vulkan_CreateSurface(window__, instance_, &surface_);
}

void Vulkan::selectPhysicalDevice() {
    vector<VkPhysicalDevice> physical_devices;
    uint32_t physical_device_count = 0;

    vkEnumeratePhysicalDevices(instance_, &physical_device_count, nullptr);
    physical_devices.resize(physical_device_count);
    vkEnumeratePhysicalDevices(instance_, &physical_device_count, physical_devices.data());

    physical_devices_ = physical_devices[0];
}

void Vulkan::selectQueueFamily() {
    vector<VkQueueFamilyProperties> queue_family_properties;
    uint32_t queue_family_count;

    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices_, &queue_family_count, nullptr);
    queue_family_properties.resize(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_devices_, &queue_family_count, queue_family_properties.data());

    int graphic_index = -1;
    int present_index = -1;

    int i = 0;
    for(const auto& queue_family : queue_family_properties) {
        if(queue_family.queueCount > 0 && queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphic_index = i;
        }

        VkBool32 present_support = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_devices_, i, surface_, &present_support);
        if(queue_family.queueCount > 0 && present_support) {
            present_index = i;
        }

        if(graphic_index != -1 && present_index != -1) {
            break;
        }

        i++;
    }

    graphics_queue_family_index_ = graphic_index;
    present_queue_family_index_ = present_index;
}

#include <set>
void Vulkan::createDevice() {
    const std::vector<const char*> device_extensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    const float queue_priority[] = { 1.0f };

    vector<VkDeviceQueueCreateInfo> queue_create_infos;
    set<uint32_t> unique_queue_families = { graphics_queue_family_index_, present_queue_family_index_ };

    float queuePriority = queue_priority[0];
    for (int queue_family : unique_queue_families) {
        VkDeviceQueueCreateInfo queue_create_info = {};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queue_family;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queuePriority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = graphics_queue_family_index_;
    queue_create_info.queueCount = 1;
    queue_create_info.pQueuePriorities = &queuePriority;

    //https://en.wikipedia.org/wiki/Anisotropic_filtering
    VkPhysicalDeviceFeatures device_features = {};
    device_features.samplerAnisotropy = VK_TRUE;

    VkDeviceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    create_info.pQueueCreateInfos = &queue_create_info;
    create_info.queueCreateInfoCount = queue_create_infos.size();
    create_info.pQueueCreateInfos = queue_create_infos.data();
    create_info.pEnabledFeatures = &device_features;
    create_info.enabledExtensionCount = device_extensions.size();
    create_info.ppEnabledExtensionNames = device_extensions.data();
    create_info.enabledLayerCount = validation_layers.size();
    create_info.ppEnabledLayerNames = validation_layers.data();

    vkCreateDevice(physical_devices_, &create_info, nullptr, &device_);
    vkGetDeviceQueue(device_, graphics_queue_family_index_, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, present_queue_family_index_, 0, &present_queue_);
}

// [Screen]
bool Vulkan::createSwapchain(bool resize) {
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_devices_, surface_,&surface_capabilities_);

    vector<VkSurfaceFormatKHR> surface_formats;
    uint32_t surface_formats_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices_, surface_,
            &surface_formats_count,
            nullptr);
    surface_formats.resize(surface_formats_count);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_devices_, surface_,
            &surface_formats_count,
            surface_formats.data());

    if (surface_formats[0].format != VK_FORMAT_B8G8R8A8_UNORM) {
        throw std::runtime_error("surfaceFormats[0].format != VK_FORMAT_B8G8R8A8_UNORM");
    }

    surface_format_ = surface_formats[0];
    int width, height = 0;
    SDL_Vulkan_GetDrawableSize(window__, &width, &height);
    width = CLAMP(width, surface_capabilities_.minImageExtent.width, surface_capabilities_.maxImageExtent.width);
    height = CLAMP(height, surface_capabilities_.minImageExtent.height, surface_capabilities_.maxImageExtent.height);
    swapchain_size_.width = width;
    swapchain_size_.height = height;

    uint32_t image_count = surface_capabilities_.minImageCount + 1;
    if (surface_capabilities_.maxImageCount > 0 && image_count > surface_capabilities_.maxImageCount) {
        image_count = surface_capabilities_.maxImageCount;
    }

    VkSwapchainCreateInfoKHR create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    create_info.surface = surface_;
    create_info.minImageCount = surface_capabilities_.minImageCount;
    create_info.imageFormat = surface_format_.format;
    create_info.imageColorSpace = surface_format_.colorSpace;
    create_info.imageExtent = swapchain_size_;
    create_info.imageArrayLayers = 1;
    create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queue_family_indices[] = {graphics_queue_family_index_, present_queue_family_index_};
    if (graphics_queue_family_index_ != present_queue_family_index_) {
        create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        create_info.queueFamilyIndexCount = 2;
        create_info.pQueueFamilyIndices = queue_family_indices;
    }
    else {
        create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    create_info.preTransform = surface_capabilities_.currentTransform;
    create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    create_info.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    create_info.clipped = VK_TRUE;

    vkCreateSwapchainKHR(device_, &create_info, nullptr, &swapchain_);
    vkGetSwapchainImagesKHR(device_, swapchain_, &swapchain_image_count_, nullptr);
    swapchain_images_.resize(swapchain_image_count_);
    vkGetSwapchainImagesKHR(device_, swapchain_, &swapchain_image_count_, swapchain_images_.data());

    return true;
}

//global CreateImageView
VkImageView Vulkan::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.image = image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = format;
    view_info.subresourceRange.aspectMask = aspectFlags;
    view_info.subresourceRange.baseMipLevel = 0;
    view_info.subresourceRange.levelCount = 1;
    view_info.subresourceRange.baseArrayLayer = 0;
    view_info.subresourceRange.layerCount = 1;

    VkImageView image_view;
    if (vkCreateImageView(device_, &view_info, nullptr, &image_view) != VK_SUCCESS) {
        throw std::runtime_error("failed to create texture image view!");
    }
    return image_view;
}

void Vulkan::createImageViews() {
    swapchain_image_views_.resize(swapchain_images_.size());

    for (uint32_t i = 0; i < swapchain_images_.size(); i++) {
        swapchain_image_views_[i] = createImageView(swapchain_images_[i], surface_format_.format, VK_IMAGE_ASPECT_COLOR_BIT);
    }    
}

VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physical_device, VkFormat *depth_format) {
    std::vector<VkFormat> depth_formats = {
        VK_FORMAT_D32_SFLOAT_S8_UINT,
        VK_FORMAT_D32_SFLOAT,
        VK_FORMAT_D24_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM_S8_UINT,
        VK_FORMAT_D16_UNORM
    };

    for (auto& format : depth_formats) {
        VkFormatProperties format_props;
        vkGetPhysicalDeviceFormatProperties(physical_device, format, &format_props);
        if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT) {
            *depth_format = format;
            return true;
        }
    }

    return false;
}

// global FindMemoryType
uint32_t Vulkan::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(physical_devices_, &mem_properties);

    for (uint32_t i = 0; i < mem_properties.memoryTypeCount; i++) {
        if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("failed to find suitable memory type!");
}

//global CreateImage
void Vulkan::createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, 
        VkDeviceMemory& image_memory) {
    VkImageCreateInfo image_info = {};
    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arrayLayers = 1;
    image_info.format = format;
    image_info.tiling = tiling;
    image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    image_info.usage = usage;
    image_info.samples = VK_SAMPLE_COUNT_1_BIT;
    image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateImage(device_, &image_info, nullptr, &image) != VK_SUCCESS) {
        throw std::runtime_error("failed to create image!");
    }

    VkMemoryRequirements mem_requirements;
    vkGetImageMemoryRequirements(device_, image, &mem_requirements);

    VkMemoryAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    alloc_info.allocationSize = mem_requirements.size;
    alloc_info.memoryTypeIndex = findMemoryType(mem_requirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &alloc_info, nullptr, &image_memory) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate image memory!");
    }

    vkBindImageMemory(device_, image, image_memory, 0);
}

void Vulkan::setupDepthStencil() {
    VkBool32 valid_depth_format = GetSupportedDepthFormat(physical_devices_, &depth_format_);
    createImage(swapchain_size_.width, swapchain_size_.height, 
            VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_TILING_OPTIMAL, 
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
            depth_image_, depth_image_memory_);
    depth_image_view_ = createImageView(depth_image_, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_IMAGE_ASPECT_DEPTH_BIT);
}

void Vulkan::createRenderPass() {
    vector<VkAttachmentDescription> attachments(2);

    attachments[0].format = surface_format_.format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    attachments[1].format = depth_format_;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass_description = {};
    subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass_description.colorAttachmentCount = 1;
    subpass_description.pColorAttachments = &color_reference;
    subpass_description.pDepthStencilAttachment = &depth_reference;
    subpass_description.inputAttachmentCount = 0;
    subpass_description.pInputAttachments = nullptr;
    subpass_description.preserveAttachmentCount = 0;
    subpass_description.pPreserveAttachments = nullptr;
    subpass_description.pResolveAttachments = nullptr;

    vector<VkSubpassDependency> dependencies(1);
    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo render_pass_info = {};
    render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_info.attachmentCount = static_cast<uint32_t>(attachments.size());
    render_pass_info.pAttachments = attachments.data();
    render_pass_info.subpassCount = 1;
    render_pass_info.pSubpasses = &subpass_description;
    render_pass_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
    render_pass_info.pDependencies = dependencies.data();

    vkCreateRenderPass(device_, &render_pass_info, nullptr, &render_pass_);
}

void Vulkan::createFramebuffers() {
    swapchain_framebuffers_.resize(swapchain_image_views_.size());

    for (size_t i = 0; i < swapchain_image_views_.size(); i++) {
        std::vector<VkImageView> attachments(2);
        attachments[0] = swapchain_image_views_[i];
        attachments[1] = depth_image_view_;

        VkFramebufferCreateInfo framebuffer_info = {};
        framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffer_info.renderPass = render_pass_;
        framebuffer_info.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebuffer_info.pAttachments = attachments.data();
        framebuffer_info.width = swapchain_size_.width;
        framebuffer_info.height = swapchain_size_.height;
        framebuffer_info.layers = 1;

        if (vkCreateFramebuffer(device_, &framebuffer_info, nullptr, &swapchain_framebuffers_[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

void Vulkan::createCommandPool() {
    VkResult result;

    VkCommandPoolCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT | VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
    create_info.queueFamilyIndex = graphics_queue_family_index_;
    vkCreateCommandPool(device_, &create_info, nullptr, &command_pool_);
}

void Vulkan::createCommandBuffers() {
    VkResult result;

    VkCommandBufferAllocateInfo allocate_info = {};
    allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocate_info.commandPool = command_pool_;
    allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocate_info.commandBufferCount = swapchain_image_count_;

    command_buffers_.resize(swapchain_image_count_);
    vkAllocateCommandBuffers(device_, &allocate_info, command_buffers_.data());
}

void Vulkan::CreateSemaphore(VkSemaphore *semaphore) {
    VkResult result;

    VkSemaphoreCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    vkCreateSemaphore(device_, &create_info, nullptr, semaphore);
}

void Vulkan::createSemaphores() {
    CreateSemaphore(&image_available_semaphore_);
    CreateSemaphore(&rendering_finished_semaphore_);
}

void Vulkan::createFences() {
    uint32_t i;
    fences_.resize(swapchain_image_count_);
    for (i = 0; i < swapchain_image_count_; i++) {
        VkResult result;
        VkFenceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        vkCreateFence(device_, &create_info, nullptr, &fences_[i]);
    }
}
