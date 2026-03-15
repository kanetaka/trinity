#pragma once
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "component.h"
#include "core/splat_types.h"
#include "core/buffer_resource.h"
#include <vector>
#include <string>
#include <memory>
#include <vulkan/vulkan.h>

class Entity;
class CommandBuffer;

class SplatComponent : public Component
{
public:
    SplatComponent(Entity* owner, const std::string& ply_file);
    ~SplatComponent() override;

    void Update(float delta_time) override;
    void Draw(std::shared_ptr<CommandBuffer>& command_buffer, VkPipelineLayout pipeline_layout);

private:
    void LoadSplats();
    void CreateBuffers();
    void CreateDescriptorSets();
    void SortSplats();

    std::string ply_file_;
    std::vector<gs::GPUSplat> gpu_splats_;
    std::vector<gs::SplatSortEntry> splat_indices_;

    std::shared_ptr<StorageBuffer> splat_buffer_;
    std::shared_ptr<StorageBuffer> index_buffer_;
    
    VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
};
