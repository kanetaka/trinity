#pragma once
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "scene/component.h"
#include "scene/component/i_renderable.h"
#include "scene/io/splat_types.h"

namespace tri
{
    class Camera;
    class StorageBuffer;
    class CommandBuffer;
    class GraphicsContext;

    // Owns a Gaussian-splat point cloud: loading, GPU buffers, camera-relative sorting and drawing.
    class SplatComponent : public IComponent, public IRenderable
    {
    public:
        SplatComponent(GraphicsContext& context, const std::string& ply_file);
        ~SplatComponent() override;

        void UpdateWithCamera(const Camera& camera, const glm::dmat4& world_transform);

        void Render(CommandBuffer& command_buffer, VkPipelineLayout pipeline_layout, uint32_t transform_index) const override;

        void SetupResources(VkDescriptorSet descriptor_set, VkBuffer ubo, VkBuffer transform_buffer) override;

        void SetDescriptorSet(VkDescriptorSet descriptor_set) { descriptor_set_ = descriptor_set; }
        const std::shared_ptr<StorageBuffer>& GetSplatBuffer() const { return splat_buffer_; }
        const std::shared_ptr<StorageBuffer>& GetIndexBuffer() const { return index_buffer_; }

    private:
        void LoadSplats();
        void CreateBuffers();
        void SortSplats(const glm::mat4& view, const glm::dmat4& world_transform, const glm::dvec3& camera_position);

        std::string ply_file_;
        std::vector<GpuSplat> gpu_splats_;
        std::vector<SplatSortEntry> splat_indices_;

        std::shared_ptr<StorageBuffer> splat_buffer_;
        std::shared_ptr<StorageBuffer> index_buffer_;
        VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
        GraphicsContext& context_;
    };
} // namespace tri
