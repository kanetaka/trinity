#pragma once
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "core/component.h"
#include "gfx/component/i_renderable.h"
#include "io/splat_types.h"

namespace tri
{
    class Camera;
    class Renderer;
    class StorageBuffer;
    class CommandBuffer;

    // Owns a Gaussian-splat point cloud: loading, GPU buffers, camera-relative sorting and drawing.
    class SplatComponent : public IComponent, public IRenderable
    {
    public:
        SplatComponent(const std::string& ply_file, Renderer* renderer);
        ~SplatComponent() override;

        void UpdateWithCamera(const Camera& camera, const glm::dmat4& world_transform);

        void Render(CommandBuffer& command_buffer, VkPipelineLayout pipeline_layout, uint32_t transform_index) const override;

    private:
        void LoadSplats();
        void CreateBuffers();
        void CreateDescriptorSets(Renderer* renderer);
        void SortSplats(const glm::mat4& view, const glm::dmat4& world_transform, const glm::dvec3& camera_position);

        std::string ply_file_;
        std::vector<GpuSplat> gpu_splats_;
        std::vector<SplatSortEntry> splat_indices_;

        std::shared_ptr<StorageBuffer> splat_buffer_;
        std::shared_ptr<StorageBuffer> index_buffer_;
        VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
    };
} // namespace tri
