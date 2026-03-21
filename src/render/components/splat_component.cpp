#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <chrono>
#include <execution>
#include <memory>

#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>

#include "render/components/splat_component.h"
#include "core/entity.h"
#include "core/ecs/components.h"
#include "app/application.h"
#include "render/renderer.h"
#include "stream/ply_loader.h"
#include "render/vulkan_context.h"
#include "core/asset_path.h"
#include "render/command_buffer.h"

namespace trinity::render {
using namespace trinity::core;
using namespace trinity::stream;

SplatComponent::SplatComponent(const std::string& ply_file, Renderer* renderer)
        : ply_file_(ply_file)
{
    LoadSplats();
    CreateBuffers();
    CreateDescriptorSets(renderer);
}

SplatComponent::~SplatComponent()
{
}

void SplatComponent::Initialize(trinity::core::ecs::Registry& registry, trinity::core::ecs::EntityId entity, Renderer* renderer)
{
    auto& data = registry.AddComponent<trinity::core::ecs::SplatDataComponent>(entity);
    data.ply_file = ply_file_;
    data.splat_buffer = splat_buffer_;
    data.index_buffer = index_buffer_;
    data.descriptor_set = descriptor_set_;
}

void SplatComponent::UpdateWithCamera(trinity::core::ecs::Registry& registry, trinity::core::ecs::EntityId entity, const trinity::core::Camera& camera)
{
    auto* data = registry.GetComponent<trinity::core::ecs::SplatDataComponent>(entity);
    if (!data) return;

    SortSplats(*data, camera.GetViewMatrix());
}

void SplatComponent::LoadSplats()
{
    std::vector<trinity::core::gs::FullSplat> splats;
    if (!trinity::stream::gs::PlyLoader::LoadPly(ply_file_, splats))
    {
        return;
    }

    gpu_splats_.reserve(splats.size());
    splat_indices_.reserve(splats.size());

    for (uint32_t i = 0; i < splats.size(); ++i)
    {
        const auto& s = splats[i];
        trinity::core::gs::GpuSplat gpu_splat;
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

    VkDeviceSize splat_size = gpu_splats_.size() * sizeof(trinity::core::gs::GpuSplat);
    splat_buffer_ = StorageBuffer::Create(splat_size, StorageBuffer::AccessMode::CpuAccessible);
    void* data = splat_buffer_->Map();
    memcpy(data, gpu_splats_.data(), splat_size);
    splat_buffer_->Unmap();

    VkDeviceSize index_size = gpu_splats_.size() * sizeof(uint32_t);
    index_buffer_ = StorageBuffer::Create(index_size, StorageBuffer::AccessMode::CpuAccessible);
}

void SplatComponent::CreateDescriptorSets(Renderer* renderer)
{
    descriptor_set_ = renderer->AllocateDescriptorSet();
    renderer->UpdateSplatDescriptorSet(descriptor_set_, splat_buffer_, index_buffer_);
}


void SplatComponent::SortSplats(trinity::core::ecs::SplatDataComponent& data, const glm::mat4& view)
{
    if (splat_indices_.empty()) return;

    // Pre-calculate depth for each splat
    for (size_t i = 0; i < splat_indices_.size(); ++i)
    {
        uint32_t initial_idx = splat_indices_[i].index;
        const auto& pos = gpu_splats_[initial_idx].position_opacity;
        glm::vec4 view_pos = view * glm::vec4(pos.x, pos.y, pos.z, 1.0f);
        splat_indices_[i].depth = view_pos.z;
    }

    std::sort(std::execution::par_unseq, splat_indices_.begin(),
        splat_indices_.end(),
        [](const trinity::core::gs::SplatSortEntry& a, const trinity::core::gs::SplatSortEntry& b)
        {
            return a.depth < b.depth;
        });

    // Upload indices to SSBO
    if (data.index_buffer)
    {
        void* buf = data.index_buffer->Map();
        uint32_t* mapped_uints = reinterpret_cast<uint32_t*>(buf);
        for (size_t i = 0; i < splat_indices_.size(); ++i)
        {
            mapped_uints[i] = splat_indices_[i].index;
        }
        data.index_buffer->Unmap();
    }
}

} // namespace trinity::render
