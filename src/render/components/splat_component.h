#pragma once
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "core/splat_types.h"

namespace trinity::core { class Camera; }
namespace trinity::core::ecs { using EntityId = uint32_t; class Registry; struct SplatDataComponent; }

namespace trinity::render {
class Renderer;
class StorageBuffer;

class SplatComponent
{
public:
    SplatComponent(const std::string& ply_file, Renderer* renderer);
    ~SplatComponent();

    void Initialize(trinity::core::ecs::Registry& registry, trinity::core::ecs::EntityId entity, Renderer* renderer);
    void UpdateWithCamera(trinity::core::ecs::Registry& registry, trinity::core::ecs::EntityId entity, const trinity::core::Camera& camera);

private:
    void LoadSplats();
    void CreateBuffers();
    void CreateDescriptorSets(Renderer* renderer);
    void SortSplats(trinity::core::ecs::SplatDataComponent& data, const glm::mat4& view);

    std::string ply_file_;
    std::vector<trinity::core::gs::GpuSplat> gpu_splats_;
    std::vector<trinity::core::gs::SplatSortEntry> splat_indices_;

    std::shared_ptr<StorageBuffer> splat_buffer_;
    std::shared_ptr<StorageBuffer> index_buffer_;
    VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
};

} // namespace trinity::render
