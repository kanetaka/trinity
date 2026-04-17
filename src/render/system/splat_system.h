#pragma once
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "io/splat_types.h"
#include "render/component/splat_data_component.h"
#include "render/resource/buffer_resource.h"

namespace tri
{
    class Camera;
    class Registry;
    using Entity = uint32_t;

    class Renderer;
    class StorageBuffer;
    struct SplatDataComponent;

    class SplatSystem
    {
    public:
        SplatSystem(const std::string& ply_file, Renderer* renderer);
        ~SplatSystem();

        void Initialize(Registry& registry, Entity entity, Renderer* renderer);
        void UpdateWithCamera(Registry& registry, Entity entity, const Camera& camera);


    private:
        void LoadSplats();
        void CreateBuffers();
        void CreateDescriptorSets(Renderer* renderer);
        void SortSplats(SplatDataComponent& data, const glm::mat4& view, const glm::dmat4& world_transform, const glm::dvec3& camera_position);

        std::string ply_file_;
        std::vector<GpuSplat> gpu_splats_;
        std::vector<SplatSortEntry> splat_indices_;

        std::shared_ptr<StorageBuffer> splat_buffer_;
        std::shared_ptr<StorageBuffer> index_buffer_;
        VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
    };
} // namespace tri
