#pragma once
#include "common/trinity_app.h"
#include "core/buffer_resource.h"
#include "core/camera.h"
#include "core/splat_types.h"
#include <memory>
#include <string>
#include <vector>

class GsApp : public ITrinityApp {
public:
    GsApp(const std::string &plyFile);
    ~GsApp() override;

    void OnInitialize() override;
    void OnDrawFrame() override;
    void OnCleanup() override;

#if defined(__ANDROID__)
    void OnSurfaceChanged() override;
#endif

    // Let the main loop pass input to us
    void ProcessInput(const Uint8 *state, float deltaTime);
    void ProcessMouseMotion(float xrel, float yrel);

private:
    void LoadSplats();
    void CreateBuffers();
    void CreateDescriptorSetLayout();
    void CreateDescriptorPool();
    void CreateDescriptorSets();
    void InitializeGraphicsPipeline();
    void SortSplats();
    void UpdateUniformBuffer();

    std::string ply_file_;
    Camera camera_;

    std::vector<gs::GPUSplat> gpu_splats_;
    std::vector<gs::SplatSortEntry> splat_indices_;

    std::shared_ptr<StorageBuffer> splat_buffer_;
    std::shared_ptr<StorageBuffer> index_buffer_;
    std::shared_ptr<UniformBuffer> uniform_buffer_;

    VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
    VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;
    VkDescriptorSet descriptor_set_ = VK_NULL_HANDLE;

    VkPipeline pipeline_ = VK_NULL_HANDLE;
    VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;

    // Viewport caching for uniform buffer
    float width_ = 1280.0f;
    float height_ = 720.0f;
};
