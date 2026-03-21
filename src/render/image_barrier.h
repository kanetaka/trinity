#pragma once

namespace tr {




class IImageResource;

struct ImageLayoutTransition
{
    VkImageLayout oldLayout;
    VkImageLayout newLayout;
    VkAccessFlags srcAccessMask;
    VkAccessFlags dstAccessMask;
    VkPipelineStageFlags srcStage;
    VkPipelineStageFlags dstStage;

    // From undefined to color attachment layout
    static ImageLayoutTransition FromUndefinedToColorAttachment();

    // From present source to color attachment layout
    static ImageLayoutTransition FromPresentSrcToColorAttachment();

    // From color attachment to present source layout
    static ImageLayoutTransition FromColorToPresent();

    // From initial layout to transfer destination
    static ImageLayoutTransition FromUndefToTransferDst();

    // From transfer destination to transfer source
    static ImageLayoutTransition FromTransferDstToTransferSrc();

    static ImageLayoutTransition ToShaderReadonlyOptimal(const IImageResource* image);

    // Transition layout and access mask to VK_IMAGE_LAYOUT_GENERAL 
    // to allow read/write access from shaders as storage image.
    // Assumes access from compute and fragment shaders,
    // ensuring dependencies for these stages.
    static ImageLayoutTransition ToStorageImageGeneralLayout(const IImageResource* image);
};


} // namespace tr
