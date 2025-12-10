#include "image_barrier.h"


ImageLayoutTransition ImageLayoutTransition::FromUndefinedToColorAttachment() {
    return {
        .old_layout = VK_IMAGE_LAYOUT_UNDEFINED,
        .new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .src_access_mask = 0,
        .dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
}

ImageLayoutTransition ImageLayoutTransition::FromPresentSrcToColorAttachment() {
    return {
        .old_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .new_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .src_access_mask = 0,
        .dst_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .src_stage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
        .dst_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    };
}

ImageLayoutTransition ImageLayoutTransition::FromColorToPresentSrc() {
    return {
        .old_layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .new_layout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .src_access_mask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        .dst_access_mask = 0,
        .src_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dst_stage = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
    };
}
