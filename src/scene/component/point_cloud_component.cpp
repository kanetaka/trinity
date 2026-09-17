#include "scene/component/point_cloud_component.h"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <execution>
#include <memory>

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "geometry/camera.h"
#include "scene/io/ply_loader.h"
#include "graphics/graphics_context.h"
#include "graphics/command_buffer.h"
#include "graphics/resource/buffer_resource.h"

using namespace tri;

namespace
{
    struct PointCloudPushConstants
    {
        uint32_t matrix_index;
        float point_size;
    };
} // namespace

PointCloudComponent::PointCloudComponent(GraphicsContext& context, const std::string& ply_file)
    : ply_file_(ply_file)
    , context_(context)
{
    LoadPointCloud();
    CreateBuffers();
}

PointCloudComponent::~PointCloudComponent()
{
}

void PointCloudComponent::PreRender(const Camera& camera, const glm::dmat4& world_transform)
{
    SortPoints(camera.GetViewMatrix(), world_transform, camera.GetPosition());
}

void PointCloudComponent::Render(CommandBuffer& command_buffer, VkPipelineLayout pipeline_layout, uint32_t transform_index) const
{
    if (descriptor_set_ == VK_NULL_HANDLE)
    {
        return;
    }

    vkCmdBindDescriptorSets(command_buffer.Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set_, 0, nullptr);

    PointCloudPushConstants pc{ transform_index, point_size_ };
    vkCmdPushConstants(command_buffer.Get(), pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(pc), &pc);

    if (index_buffer_)
    {
        uint32_t num_points = static_cast<uint32_t>(index_buffer_->GetBufferSize() / sizeof(uint32_t));
        // Draw quad billboard polygon (6 vertices = 2 triangles) per point via GPU instancing
        vkCmdDraw(command_buffer.Get(), 6, num_points, 0, 0);
    }
}

void PointCloudComponent::LoadPointCloud()
{
    std::vector<PointCloudVertex> vertices;
    if (!PlyLoader::LoadPointCloud(ply_file_, vertices))
    {
        return;
    }

    if (vertices.empty())
    {
        return;
    }

    // Compute bounding box to estimate an appropriate initial point size
    glm::vec3 min_pos = vertices[0].position;
    glm::vec3 max_pos = vertices[0].position;
    for (const auto& v : vertices)
    {
        min_pos = glm::min(min_pos, v.position);
        max_pos = glm::max(max_pos, v.position);
    }
    float diag = glm::length(max_pos - min_pos);
    float estimated_radius = diag / (2.0f * std::sqrt(static_cast<float>(vertices.size())));
    if (estimated_radius <= 0.0f || std::isnan(estimated_radius))
    {
        estimated_radius = 0.015f;
    }
    point_size_ = std::clamp(estimated_radius, 0.002f, 0.1f);

    gpu_points_.reserve(vertices.size());
    point_indices_.reserve(vertices.size());

    for (uint32_t i = 0; i < vertices.size(); ++i)
    {
        const auto& v = vertices[i];
        GpuPoint gpu_point;
        gpu_point.position = glm::vec4(v.position, 1.0f);
        gpu_point.color = glm::vec4(v.color, 1.0f);

        gpu_points_.push_back(gpu_point);
        point_indices_.push_back({ i, 0.0f });
    }
}

void PointCloudComponent::CreateBuffers()
{
    if (gpu_points_.empty())
    {
        return;
    }

    VkDeviceSize point_size = gpu_points_.size() * sizeof(GpuPoint);
    point_cloud_buffer_ = StorageBuffer::Create(context_, point_size, StorageBuffer::AccessMode::CpuAccessible);
    void* data = point_cloud_buffer_->Map();
    memcpy(data, gpu_points_.data(), point_size);
    point_cloud_buffer_->Unmap();

    VkDeviceSize index_size = gpu_points_.size() * sizeof(uint32_t);
    index_buffer_ = StorageBuffer::Create(context_, index_size, StorageBuffer::AccessMode::CpuAccessible);
}

void PointCloudComponent::SetupResources(VkDescriptorSet descriptor_set, VkBuffer ubo, VkBuffer transform_buffer)
{
    descriptor_set_ = descriptor_set;
    if (descriptor_set_ == VK_NULL_HANDLE || !point_cloud_buffer_ || !index_buffer_)
    {
        return;
    }

    auto device = context_.GetVkDevice();

    VkDescriptorBufferInfo ubo_info{};
    ubo_info.buffer = ubo;
    ubo_info.offset = 0;
    ubo_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet ubo_write{};
    ubo_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    ubo_write.dstSet = descriptor_set_;
    ubo_write.dstBinding = 0;
    ubo_write.dstArrayElement = 0;
    ubo_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubo_write.descriptorCount = 1;
    ubo_write.pBufferInfo = &ubo_info;

    VkDescriptorBufferInfo point_info{};
    point_info.buffer = point_cloud_buffer_->GetVkBuffer();
    point_info.offset = 0;
    point_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet point_write{};
    point_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    point_write.dstSet = descriptor_set_;
    point_write.dstBinding = 1;
    point_write.dstArrayElement = 0;
    point_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    point_write.descriptorCount = 1;
    point_write.pBufferInfo = &point_info;

    VkDescriptorBufferInfo idx_info{};
    idx_info.buffer = index_buffer_->GetVkBuffer();
    idx_info.offset = 0;
    idx_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet idx_write{};
    idx_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    idx_write.dstSet = descriptor_set_;
    idx_write.dstBinding = 2;
    idx_write.dstArrayElement = 0;
    idx_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    idx_write.descriptorCount = 1;
    idx_write.pBufferInfo = &idx_info;

    VkDescriptorBufferInfo transform_info{};
    transform_info.buffer = transform_buffer;
    transform_info.offset = 0;
    transform_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet transform_write{};
    transform_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    transform_write.dstSet = descriptor_set_;
    transform_write.dstBinding = 3;
    transform_write.dstArrayElement = 0;
    transform_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    transform_write.descriptorCount = 1;
    transform_write.pBufferInfo = &transform_info;

    std::vector<VkWriteDescriptorSet> writes = { ubo_write, point_write, idx_write, transform_write };
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}

void PointCloudComponent::SortPoints(const glm::mat4& view, const glm::dmat4& world_transform, const glm::dvec3& camera_position)
{
    if (point_indices_.empty())
    {
        return;
    }

    glm::dmat4 rel = world_transform;
    rel[3][0] -= camera_position.x;
    rel[3][1] -= camera_position.y;
    rel[3][2] -= camera_position.z;
    glm::mat4 rel_model = glm::mat4(rel);
    glm::mat4 mv = view * rel_model;

    // Pre-calculate depth for each point
    for (size_t i = 0; i < point_indices_.size(); ++i)
    {
        uint32_t initial_idx = point_indices_[i].index;
        const auto& pos = gpu_points_[initial_idx].position;
        glm::vec4 view_pos = mv * glm::vec4(pos.x, pos.y, pos.z, 1.0f);
        point_indices_[i].depth = view_pos.z;
    }

    std::sort(std::execution::par, point_indices_.begin(),
        point_indices_.end(),
        [](const PointSortEntry& a, const PointSortEntry& b)
        {
            if (std::isnan(a.depth)) return false;
            if (std::isnan(b.depth)) return true;
            return a.depth < b.depth;
        });

    // Upload indices to SSBO
    if (index_buffer_)
    {
        void* buf = index_buffer_->Map();
        uint32_t* mapped_uints = reinterpret_cast<uint32_t*>(buf);
        for (size_t i = 0; i < point_indices_.size(); ++i)
        {
            mapped_uints[i] = point_indices_[i].index;
        }
        index_buffer_->Unmap();
    }
}
