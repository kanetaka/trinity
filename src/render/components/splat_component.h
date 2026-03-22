#pragma once
#include <string>
#include <vector>
#include <memory>
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include "stream/splat_types.h"
#include "render/components/splat_data_component.h"
#include "render/resources/buffer_resource.h"

namespace tri
{
    class Camera;
    class Registry;
    using EntityId = uint32_t;

    class Renderer;
    class StorageBuffer;
    struct SplatDataComponent;

    class SplatComponent
    {
    public:
        SplatComponent(const std::string& ply_file, Renderer* renderer);
        ~SplatComponent();

        void Initialize(Registry& registry, EntityId entity, Renderer* renderer);
        void UpdateWithCamera(Registry& registry, EntityId entity, const Camera& camera);


    private:
        void LoadSplats();
        void CreateBuffers();
        void CreateDescriptorSets(Renderer* renderer);
        void SortSplats(SplatDataComponent& data, const glm::mat4& view);

        std::string ply_file_;
        std::vector<GpuSplat> gpu_splats_;
        std::vector<SplatSortEntry> splat_indices_;

        std::shared_ptr<StorageBuffer> splat_buffer_;
        std::shared_ptr<StorageBuffer> index_buffer_;
        VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
    };
} // namespace tri
