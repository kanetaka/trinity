#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <execution>
#include <memory>

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>

#include "scene/component/splat_component.h"
#include "geometry/camera.h"
#include "scene/io/ply_loader.h"
#include "graphics/graphics_context.h"
#include "core/asset_path.h"
#include "graphics/command_buffer.h"
#include "graphics/resource/buffer_resource.h"

using namespace tri;

SplatComponent::SplatComponent(GraphicsContext& context, const std::string& ply_file)
    : ply_file_(ply_file)
    , context_(context)
{
    LoadSplats();
    CreateBuffers();
}

SplatComponent::~SplatComponent()
{
}

void SplatComponent::PreRender(const Camera& camera, const glm::dmat4& world_transform)
{
    SortSplats(camera.GetViewMatrix(), world_transform, camera.GetPosition());
}

void SplatComponent::Render(CommandBuffer& command_buffer, VkPipelineLayout pipeline_layout, uint32_t transform_index) const
{
    if (descriptor_set_ == VK_NULL_HANDLE) return;

    vkCmdBindDescriptorSets(command_buffer.Get(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set_, 0, nullptr);
    vkCmdPushConstants(command_buffer.Get(), pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uint32_t), &transform_index);

    if (index_buffer_)
    {
        uint32_t num_splats = static_cast<uint32_t>(index_buffer_->GetBufferSize() / sizeof(uint32_t));
        vkCmdDraw(command_buffer.Get(), 4, num_splats, 0, 0);
    }
}

void SplatComponent::LoadSplats()
{
    std::vector<FullSplat> splats;
    if (!PlyLoader::LoadPly(ply_file_, splats))
    {
        return;
    }

    gpu_splats_.reserve(splats.size());
    splat_indices_.reserve(splats.size());

    for (uint32_t i = 0; i < splats.size(); ++i)
    {
        const auto& s = splats[i];
        GpuSplat gpu_splat;
        gpu_splat.position_opacity = glm::vec4(s.position, s.opacity);
        gpu_splat.rot_scale_0 = glm::vec4(s.rot.x, s.rot.y, s.rot.z, s.scale.x);
        gpu_splat.rot_w_scale_yz = glm::vec4(s.rot.w, s.scale.y, s.scale.z, 0.0f);
        gpu_splat.sh_dc = glm::vec4(s.sh_dc[0], s.sh_dc[1], s.sh_dc[2], 0.0f);

        gpu_splats_.push_back(gpu_splat);
        splat_indices_.push_back({i, 0.0f});
    }
}

void SplatComponent::CreateBuffers()
{
    if (gpu_splats_.empty()) return;

    VkDeviceSize splat_size = gpu_splats_.size() * sizeof(GpuSplat);
    splat_buffer_ = StorageBuffer::Create(context_, splat_size, StorageBuffer::AccessMode::CpuAccessible);
    void* data = splat_buffer_->Map();
    memcpy(data, gpu_splats_.data(), splat_size);
    splat_buffer_->Unmap();

    VkDeviceSize index_size = gpu_splats_.size() * sizeof(uint32_t);
    index_buffer_ = StorageBuffer::Create(context_, index_size, StorageBuffer::AccessMode::CpuAccessible);
}

void SplatComponent::SetupResources(VkDescriptorSet descriptor_set, VkBuffer ubo, VkBuffer transform_buffer)
{
    descriptor_set_ = descriptor_set;
    if (descriptor_set_ == VK_NULL_HANDLE || !splat_buffer_ || !index_buffer_)
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

    VkDescriptorBufferInfo splat_info{};
    splat_info.buffer = splat_buffer_->GetVkBuffer();
    splat_info.offset = 0;
    splat_info.range = VK_WHOLE_SIZE;

    VkWriteDescriptorSet splat_write{};
    splat_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    splat_write.dstSet = descriptor_set_;
    splat_write.dstBinding = 1;
    splat_write.dstArrayElement = 0;
    splat_write.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    splat_write.descriptorCount = 1;
    splat_write.pBufferInfo = &splat_info;

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

    std::vector<VkWriteDescriptorSet> writes = { ubo_write, splat_write, idx_write, transform_write };
    vkUpdateDescriptorSets(device, static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);
}


void SplatComponent::SortSplats(const glm::mat4& view, const glm::dmat4& world_transform, const glm::dvec3& camera_position)
{
    if (splat_indices_.empty()) return;

    glm::dmat4 rel = world_transform;
    rel[3][0] -= camera_position.x;
    rel[3][1] -= camera_position.y;
    rel[3][2] -= camera_position.z;
    glm::mat4 rel_model = glm::mat4(rel);
    glm::mat4 mv = view * rel_model;

    // Pre-calculate depth for each splat
    for (size_t i = 0; i < splat_indices_.size(); ++i)
    {
        uint32_t initial_idx = splat_indices_[i].index;
        const auto& pos = gpu_splats_[initial_idx].position_opacity;
        glm::vec4 view_pos = mv * glm::vec4(pos.x, pos.y, pos.z, 1.0f);
        splat_indices_[i].depth = view_pos.z;
    }

    std::sort(std::execution::par, splat_indices_.begin(),
        splat_indices_.end(),
        [](const SplatSortEntry& a, const SplatSortEntry& b)
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
        for (size_t i = 0; i < splat_indices_.size(); ++i)
        {
            mapped_uints[i] = splat_indices_[i].index;
        }
        index_buffer_->Unmap();
    }
}
