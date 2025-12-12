#pragma once
#include "common/application.h"
#include "core/vulkan_context.h"
#include "core/swapchain.h"
#include "core/buffer_resource.h"
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <array>
#include <stdexcept>

class TriangleApp : public IApplication {
public:
    virtual void Initialize() override;
    virtual void DrawFrame() override;
    virtual void Cleanup() override;

    struct Vertex {
        glm::vec3 position;
        glm::vec3 color;
    };

private:
    void InitializeTriangleVertexBuffer();
    void InitializeGraphicsPipeline();

private:
    std::shared_ptr<VertexBuffer> vertex_buffer_;
    VkPipeline pipeline_ = VK_NULL_HANDLE;
	VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;
};
