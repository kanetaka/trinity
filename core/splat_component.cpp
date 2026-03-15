#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include "core/splat_component.h"
#include "core/entity.h"
#include "gs_app.h"
#include "core/renderer.h"
#include "core/ply_loader.h"
#include "core/vulkan_context.h"
#include "core/asset_path.h"
#include "core/command_buffer.h"
#include <SDL3/SDL.h>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <execution>
#include <glm/gtc/matrix_transform.hpp>

SplatComponent::SplatComponent(Entity* owner, const std::string& ply_file)
        : Component(owner), ply_file_(ply_file)
{
    LoadSplats();
    CreateBuffers();
    CreateDescriptorSets();
    
    owner_->GetGame()->GetRenderer()->AddComponent(this);
}

SplatComponent::~SplatComponent()
{
    if (owner_ && owner_->GetGame() && owner_->GetGame()->GetRenderer())
    {
        owner_->GetGame()->GetRenderer()->RemoveComponent(this);
    }
}

void SplatComponent::Update(float delta_time)
{
    SortSplats();
}

void SplatComponent::Draw(std::shared_ptr<CommandBuffer>& command_buffer, VkPipelineLayout pipeline_layout)
{
    if (splat_indices_.empty() || descriptor_set_ == VK_NULL_HANDLE)
    {
        return;
    }

    vkCmdBindDescriptorSets(*command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                                    pipeline_layout, 0, 1, &descriptor_set_, 0, nullptr);

    // Draw 4 vertices (quad) for each splat
    vkCmdDraw(*command_buffer, 4, static_cast<uint32_t>(splat_indices_.size()), 0, 0);
}

void SplatComponent::LoadSplats()
{
    std::vector<gs::FullSplat> splats;
    if (!gs::PlyLoader::LoadPly(ply_file_, splats))
    {
        return;
    }

    gpu_splats_.reserve(splats.size());
    splat_indices_.reserve(splats.size());

    for (uint32_t i = 0; i < splats.size(); ++i)
    {
        const auto& s = splats[i];
        gs::GPUSplat gpu_splat;
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

    VkDeviceSize splat_size = gpu_splats_.size() * sizeof(gs::GPUSplat);
    splat_buffer_ = StorageBuffer::Create(splat_size, StorageBuffer::AccessMode::CPUAccessible);
    
    void* data = splat_buffer_->Map();
    memcpy(data, gpu_splats_.data(), splat_size);
    splat_buffer_->Unmap();

    VkDeviceSize index_size = splat_indices_.size() * sizeof(uint32_t);
    index_buffer_ = StorageBuffer::Create(index_size, StorageBuffer::AccessMode::CPUAccessible);
}

void SplatComponent::CreateDescriptorSets()
{
    auto renderer = owner_->GetGame()->GetRenderer();
    descriptor_set_ = renderer->AllocateDescriptorSet();
    if (descriptor_set_ != VK_NULL_HANDLE)
    {
        renderer->UpdateSplatDescriptorSet(descriptor_set_, splat_buffer_, index_buffer_);
    }
}

void SplatComponent::SortSplats()
{
    if (splat_indices_.empty()) return;

    // Get view matrix from GsApp/Renderer
    glm::mat4 view = owner_->GetGame()->GetCamera().GetViewMatrix();
    
    // Simple CPU sort (as in original GsApp)
    for (size_t i = 0; i < splat_indices_.size(); ++i)
    {
        uint32_t initial_idx = splat_indices_[i].index;
        const auto& pos = gpu_splats_[initial_idx].position_opacity;
        glm::vec4 view_pos = view * glm::vec4(pos.x, pos.y, pos.z, 1.0f);
        splat_indices_[i].depth = view_pos.z;
    }

	std::sort(std::execution::par_unseq, splat_indices_.begin(),
		splat_indices_.end(),
		[](const gs::SplatSortEntry& a, const gs::SplatSortEntry& b)
		{
			return a.depth < b.depth;
		});

    // Upload indices to SSBO
    if (index_buffer_)
    {
        void* data = index_buffer_->Map();
        uint32_t* mapped_uints = reinterpret_cast<uint32_t*>(data);
        for (size_t i = 0; i < splat_indices_.size(); ++i)
        {
            mapped_uints[i] = splat_indices_[i].index;
        }
        index_buffer_->Unmap();
    }
}
