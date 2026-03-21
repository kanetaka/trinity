#pragma once
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#endif
#include "core/component.h"
#include "core/splat_types.h"
#include "render/resources/buffer_resource.h"
#include <vector>
#include <string>
#include <memory>
#include <vulkan/vulkan.h>

namespace trinity::core { class Camera; }
namespace trinity::render {



class CommandBuffer;
class Renderer;
class Camera;

class SplatComponent : public trinity::core::Component
{
public:
    SplatComponent(trinity::core::Entity* owner, const std::string& ply_file, Renderer* renderer);
    ~SplatComponent() override;

    void Update(float delta_time) override;
    void UpdateWithCamera(float delta_time, const trinity::core::Camera& camera);
    void Draw(std::shared_ptr<CommandBuffer>& command_buffer, VkPipelineLayout pipeline_layout);

private:
    void LoadSplats();
    void CreateBuffers();
    void CreateDescriptorSets(Renderer* renderer);
    void SortSplats(const glm::mat4& view);

    std::string ply_file_;
    std::vector<trinity::core::gs::GpuSplat> gpu_splats_;
    std::vector<trinity::core::gs::SplatSortEntry> splat_indices_;

    std::shared_ptr<StorageBuffer> splat_buffer_;
    std::shared_ptr<StorageBuffer> index_buffer_;

    VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
};


} // namespace trinity::render
