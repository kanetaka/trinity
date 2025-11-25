#include "VulkanFunction.h"
#include "VulkanExtern.h"
extern Vulkan *vulkan;

uint32_t frameIndex;
VkCommandBuffer commandBuffer;
VkImage image;
void acquireNextImage() {
    vkAcquireNextImageKHR(vulkan->device_, vulkan->swapchain_, UINT64_MAX, vulkan->image_available_semaphore_, VK_NULL_HANDLE, &frameIndex);

    vkWaitForFences(vulkan->device_, 1, &vulkan->fences_[frameIndex], VK_FALSE, UINT64_MAX);
    vkResetFences(vulkan->device_, 1, &vulkan->fences_[frameIndex]);  

    commandBuffer = vulkan->command_buffers_[frameIndex];
    image = vulkan->swapchain_images_[frameIndex];
}

void resetCommandBuffer() {
    vkResetCommandBuffer(commandBuffer, 0);
}

void beginCommandBuffer() {
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
}

void endCommandBuffer() {
    vkEndCommandBuffer(commandBuffer);
}

void freeCommandBuffers() {
    vkFreeCommandBuffers(vulkan->device_, vulkan->command_pool_, 1, &commandBuffer);
}

void beginRenderPass(VkClearColorValue clear_color,VkClearDepthStencilValue clear_depth_stencil) {
    VkRenderPassBeginInfo render_pass_info = {};
    render_pass_info.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_info.renderPass        = vulkan->render_pass_;
    render_pass_info.framebuffer       = vulkan->swapchain_framebuffers_[frameIndex];
    render_pass_info.renderArea.offset = {0, 0};
    render_pass_info.renderArea.extent = vulkan->swapchain_size_;
    render_pass_info.clearValueCount   = 1;

    vector<VkClearValue> clearValues(2);
    clearValues[0].color = clear_color;
    clearValues[1].depthStencil = clear_depth_stencil;

    render_pass_info.clearValueCount = static_cast<uint32_t>(clearValues.size());
    render_pass_info.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
}

void endRenderPass() {
    vkCmdEndRenderPass(commandBuffer);
}

VkPipelineStageFlags waitDestStageMask = VK_PIPELINE_STAGE_TRANSFER_BIT;
void queueSubmit() {
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &vulkan->image_available_semaphore_;
    submitInfo.pWaitDstStageMask = &waitDestStageMask;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &vulkan->rendering_finished_semaphore_;
    vkQueueSubmit(vulkan->graphics_queue_, 1, &submitInfo, vulkan->fences_[frameIndex]);
}

void queuePresent() {
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &vulkan->rendering_finished_semaphore_;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &vulkan->swapchain_;
    presentInfo.pImageIndices = &frameIndex;
    vkQueuePresentKHR(vulkan->present_queue_, &presentInfo);

    vkQueueWaitIdle(vulkan->present_queue_);
}

void setViewport(int width,int height) {
    VkViewport viewport;
    viewport.width = (float)width / 2;
    viewport.height = (float)height;
    viewport.minDepth = (float)0.0f;
    viewport.maxDepth = (float)1.0f;
    viewport.x = 0;
    viewport.y = 0;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
}

void setScissor(int width,int height) {
    VkRect2D scissor;
    scissor.extent.width = width / 2;
    scissor.extent.height = height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void drawTriangle() {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vulkan->graphicsPipeline_);
    
    VkBuffer vertexBuffers[] = {vulkan->vertexBuffer_};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);
}