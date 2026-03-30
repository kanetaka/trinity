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
        void SortSplats(SplatDataComponent& data, const glm::mat4& view);

        std::string ply_file_;
        std::vector<GpuSplat> gpu_splats_;
        std::vector<SplatSortEntry> splat_indices_;

        std::shared_ptr<StorageBuffer> splat_buffer_;
        std::shared_ptr<StorageBuffer> index_buffer_;
        VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;
    };
} // namespace tri
