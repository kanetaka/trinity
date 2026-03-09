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

  std::string m_plyFile;
  Camera m_camera;

  std::vector<gs::GPUSplat> m_gpuSplats;
  std::vector<gs::SplatSortEntry> m_splatIndices;

  std::shared_ptr<StorageBuffer> m_splatBuffer;
  std::shared_ptr<StorageBuffer> m_indexBuffer;
  std::shared_ptr<UniformBuffer> m_uniformBuffer;

  VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
  VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
  VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;

  VkPipeline m_pipeline = VK_NULL_HANDLE;
  VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

  // Viewport caching for uniform buffer
  float m_width = 1280.0f;
  float m_height = 720.0f;
};
