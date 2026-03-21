#pragma once
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "stream/splat_types.h"
#include "render/components/splat_data_component.h"
#include "render/resources/buffer_resource.h"

namespace trinity::core { class Camera; }
namespace trinity::core { using EntityId = uint32_t; class Registry; }

namespace trinity::render {
class Renderer;
class StorageBuffer;

class SplatComponent
{
public:
    SplatComponent(const std::string& ply_file, Renderer* renderer);
    ~SplatComponent();

    void Initialize(trinity::core::Registry& registry, trinity::core::EntityId entity, Renderer* renderer);
    void UpdateWithCamera(trinity::core::Registry& registry, trinity::core::EntityId entity, const trinity::core::Camera& camera);


private:
    void LoadSplats();
    void CreateBuffers();
    void CreateDescriptorSets(Renderer* renderer);
    void SortSplats(trinity::render::SplatDataComponent& data, const glm::mat4& view);

    std::string ply_file_;
    std::vector<trinity::stream::gs::GpuSplat> gpu_splats_;
    std::vector<trinity::stream::gs::SplatSortEntry> splat_indices_;

    std::shared_ptr<StorageBuffer> splat_buffer_;
    std::shared_ptr<StorageBuffer> index_buffer_;
    VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
};

} // namespace trinity::render
