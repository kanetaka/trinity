#pragma once
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "scene/component.h"
#include "scene/component/i_renderable.h"
#include "scene/component/i_pre_renderable.h"
#include "scene/io/point_cloud_types.h"

namespace tri
{
    class Camera;
    class StorageBuffer;
    class CommandBuffer;
    class GraphicsContext;

    // Component that represents and renders a point cloud loaded from a PLY file.
    // Renders camera-facing quad billboard polygons via GPU instancing.
    class PointCloudComponent : public IComponent, public IRenderable, public IPreRenderable
    {
    public:
        PointCloudComponent(GraphicsContext& context, const std::string& ply_file);
        ~PointCloudComponent() override;

        void PreRender(const Camera& camera, const glm::dmat4& world_transform) override;

        void Render(CommandBuffer& command_buffer, VkPipelineLayout pipeline_layout, uint32_t transform_index) const override;

        void SetupResources(VkDescriptorSet descriptor_set, VkBuffer ubo, VkBuffer transform_buffer) override;

        void SetPointSize(float size) { point_size_ = size; }
        float GetPointSize() const { return point_size_; }
        float* GetPointSizePtr() { return &point_size_; }

        void SetDescriptorSet(VkDescriptorSet descriptor_set) { descriptor_set_ = descriptor_set; }
        const std::shared_ptr<StorageBuffer>& GetPointCloudBuffer() const { return point_cloud_buffer_; }
        const std::shared_ptr<StorageBuffer>& GetIndexBuffer() const { return index_buffer_; }

    private:
        void LoadPointCloud();
        void CreateBuffers();
        void SortPoints(const glm::mat4& view, const glm::dmat4& world_transform, const glm::dvec3& camera_position);

        std::string ply_file_;
        std::vector<GpuPoint> gpu_points_;
        std::vector<PointSortEntry> point_indices_;

        std::shared_ptr<StorageBuffer> point_cloud_buffer_;
        std::shared_ptr<StorageBuffer> index_buffer_;
        VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
        GraphicsContext& context_;
        float point_size_ = 0.015f;
    };
} // namespace tri

