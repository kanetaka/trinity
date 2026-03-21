#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#endif
#include <glm/glm.hpp>
#include "core/entity.h"

namespace trinity::core { class Registry; }
class Application;

namespace trinity::render {


class CommandBuffer;
class UniformBuffer;
class StorageBuffer;



class Renderer
{
public:
    Renderer(Application* app);
    ~Renderer();

    bool Initialize(float screen_width, float screen_height);
    void Shutdown();

    void Draw(trinity::core::Entity* root);

    void SetViewMatrix(const glm::mat4& view) { view_ = view; }
    void SetProjectionMatrix(const glm::mat4& proj) { projection_ = proj; }

    void UpdateUniformBuffer();
    void UpdateTransformBuffer(trinity::core::Registry& registry);

    VkDescriptorSetLayout GetSplatDescriptorSetLayout() const { return descriptor_set_layout_; }
    VkDescriptorSet AllocateDescriptorSet();
    void UpdateSplatDescriptorSet(VkDescriptorSet set, const std::shared_ptr<StorageBuffer>& splat_buffer, const std::shared_ptr<StorageBuffer>& index_buffer);

    float GetScreenWidth() const { return screen_width_; }
    float GetScreenHeight() const { return screen_height_; }

private:
    bool CreateDescriptorSetLayout();
    bool CreateDescriptorPool();
    bool CreateDescriptorSets();
    bool InitializeGraphicsPipeline();
    void DrawEntity(trinity::core::Entity* entity, std::shared_ptr<CommandBuffer>& command_buffer);

    Application* app_;

    VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;

    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;

    std::shared_ptr<UniformBuffer> uniform_buffer_;
    std::shared_ptr<StorageBuffer> transform_buffer_;

    glm::mat4 view_;
    glm::mat4 projection_;
    float screen_width_;
    float screen_height_;
};


} // namespace trinity::render
