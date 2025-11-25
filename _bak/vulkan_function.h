#pragma once

#include <vulkan/vulkan.h>

void acquireNextImage();

void resetCommandBuffer();
void beginCommandBuffer();
void endCommandBuffer();
void freeCommandBuffers();

void beginRenderPass(VkClearColorValue clear_color,VkClearDepthStencilValue clear_depth_stencil);
void endRenderPass();

void queueSubmit();
void queuePresent();

void setViewport(int width,int height);
void setScissor(int width,int height);

void drawTriangle();
