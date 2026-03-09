#include "gs_app.h"
#include "core/asset_path.h"
#include "core/graphics_pipeline_builder.h"
#include "core/ply_loader.h"
#include "core/shader_loader.h"
#include "core/swapchain.h"
#include <algorithm>
#include <execution>
#include <iostream>

struct CameraUBO {
  glm::mat4 view;
  glm::mat4 proj;
  glm::vec2 viewport;
  glm::vec2 padding;
};

GsApp::GsApp(const std::string &plyFile)
    : m_plyFile(plyFile), m_camera(glm::vec3(0.0f, 0.0f, 5.0f),
                                   glm::vec3(0.0f, -1.0f, 0.0f), -90.0f, 0.0f) {
}

GsApp::~GsApp() {}

void GsApp::OnInitialize() {
  LoadSplats();
  CreateBuffers();
  CreateDescriptorSetLayout();
  CreateDescriptorPool();
  CreateDescriptorSets();
  InitializeGraphicsPipeline();
}

void GsApp::LoadSplats() {
  std::vector<gs::FullSplat> splats;
  if (!gs::PlyLoader::LoadPly(m_plyFile, splats)) {
    std::cerr << "Failed to load PLY. Using empty splat list." << std::endl;
    return;
  }

  m_gpuSplats.reserve(splats.size());
  m_splatIndices.reserve(splats.size());

  // Compute scene bounds
  glm::vec3 min_pt(std::numeric_limits<float>::max());
  glm::vec3 max_pt(std::numeric_limits<float>::lowest());

  for (uint32_t i = 0; i < splats.size(); ++i) {
    const auto &s = splats[i];
    gs::GPUSplat gpuSplat;
    gpuSplat.position_opacity = glm::vec4(s.position, s.opacity);
    gpuSplat.rot_scale_0 = glm::vec4(s.rot.x, s.rot.y, s.rot.z, s.scale.x);
    gpuSplat.rot_w_scale_yz = glm::vec4(s.rot.w, s.scale.y, s.scale.z, 0.0f);
    gpuSplat.sh_dc = glm::vec4(s.sh_dc[0], s.sh_dc[1], s.sh_dc[2], 0.0f);

    m_gpuSplats.push_back(gpuSplat);
    m_splatIndices.push_back({i, 0.0f});

    min_pt = glm::min(min_pt, s.position);
    max_pt = glm::max(max_pt, s.position);
  }

  // Auto-center camera
  glm::vec3 center = (min_pt + max_pt) * 0.5f;
  float radius = glm::length(max_pt - center);
  m_camera.Position = center + glm::vec3(0.0f, 0.0f, radius * 1.5f);
}

void GsApp::CreateBuffers() {
  auto &vulkanCtx = VulkanContext::Get();

  // 1. Uniform Buffer
  m_uniformBuffer = UniformBuffer::Create(sizeof(CameraUBO));

  // 2. Splat Storage Buffer (Read-only on GPU)
  if (!m_gpuSplats.empty()) {
    VkDeviceSize splatBufferSize = m_gpuSplats.size() * sizeof(gs::GPUSplat);
    m_splatBuffer = StorageBuffer::Create(
        splatBufferSize, StorageBuffer::AccessMode::CPUAccessible);

    void *data = m_splatBuffer->Map();
    memcpy(data, m_gpuSplats.data(), splatBufferSize);
    m_splatBuffer->Unmap();

    // 3. Index Buffer (Storage Buffer)
    VkDeviceSize indexBufferSize = m_splatIndices.size() * sizeof(uint32_t);
    m_indexBuffer = StorageBuffer::Create(
        indexBufferSize, StorageBuffer::AccessMode::CPUAccessible);
  }
}

void GsApp::CreateDescriptorSetLayout() {
  auto device = VulkanContext::Get().GetVkDevice();

  VkDescriptorSetLayoutBinding uboBinding{};
  uboBinding.binding = 0;
  uboBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboBinding.descriptorCount = 1;
  uboBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding splatBinding{};
  splatBinding.binding = 1;
  splatBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  splatBinding.descriptorCount = 1;
  splatBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  VkDescriptorSetLayoutBinding idxBinding{};
  idxBinding.binding = 2;
  idxBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
  idxBinding.descriptorCount = 1;
  idxBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

  std::vector<VkDescriptorSetLayoutBinding> bindings = {
      uboBinding, splatBinding, idxBinding};
  VkDescriptorSetLayoutCreateInfo layoutInfo{};
  layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
  layoutInfo.pBindings = bindings.data();

  if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr,
                                  &m_descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

void GsApp::CreateDescriptorPool() {
  auto device = VulkanContext::Get().GetVkDevice();

  std::vector<VkDescriptorPoolSize> poolSizes = {
      {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
      {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2}};

  VkDescriptorPoolCreateInfo poolInfo{};
  poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  poolInfo.pPoolSizes = poolSizes.data();
  poolInfo.maxSets = 1;

  if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &m_descriptorPool) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

void GsApp::CreateDescriptorSets() {
  auto device = VulkanContext::Get().GetVkDevice();

  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = m_descriptorPool;
  allocInfo.descriptorSetCount = 1;
  allocInfo.pSetLayouts = &m_descriptorSetLayout;

  if (vkAllocateDescriptorSets(device, &allocInfo, &m_descriptorSet) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to allocate descriptor sets!");
  }

  VkDescriptorBufferInfo uboInfo{};
  uboInfo.buffer = m_uniformBuffer->GetVkBuffer();
  uboInfo.offset = 0;
  uboInfo.range = sizeof(CameraUBO);

  VkWriteDescriptorSet uboWrite{};
  uboWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  uboWrite.dstSet = m_descriptorSet;
  uboWrite.dstBinding = 0;
  uboWrite.dstArrayElement = 0;
  uboWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
  uboWrite.descriptorCount = 1;
  uboWrite.pBufferInfo = &uboInfo;

  std::vector<VkWriteDescriptorSet> descriptorWrites = {uboWrite};

  VkDescriptorBufferInfo splatInfo{};
  VkWriteDescriptorSet splatWrite{};
  VkDescriptorBufferInfo idxInfo{};
  VkWriteDescriptorSet idxWrite{};

  if (m_splatBuffer && m_indexBuffer) {
    splatInfo.buffer = m_splatBuffer->GetVkBuffer();
    splatInfo.offset = 0;
    splatInfo.range = VK_WHOLE_SIZE;

    splatWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    splatWrite.dstSet = m_descriptorSet;
    splatWrite.dstBinding = 1;
    splatWrite.dstArrayElement = 0;
    splatWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    splatWrite.descriptorCount = 1;
    splatWrite.pBufferInfo = &splatInfo;

    idxInfo.buffer = m_indexBuffer->GetVkBuffer();
    idxInfo.offset = 0;
    idxInfo.range = VK_WHOLE_SIZE;

    idxWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    idxWrite.dstSet = m_descriptorSet;
    idxWrite.dstBinding = 2;
    idxWrite.dstArrayElement = 0;
    idxWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    idxWrite.descriptorCount = 1;
    idxWrite.pBufferInfo = &idxInfo;

    descriptorWrites.push_back(splatWrite);
    descriptorWrites.push_back(idxWrite);
  }

  vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()),
                         descriptorWrites.data(), 0, nullptr);
}

void GsApp::InitializeGraphicsPipeline() {
  auto &vulkanCtx = VulkanContext::Get();
  auto device = vulkanCtx.GetVkDevice();
  auto extent = vulkanCtx.GetSwapchain()->GetExtent();

  m_width = (float)extent.width;
  m_height = (float)extent.height;

  // Load Shaders
  auto vertModule =
      loader::LoadShaderModule(GetAssetRootPath() / "splat.vert.spv");
  auto fragModule =
      loader::LoadShaderModule(GetAssetRootPath() / "splat.frag.spv");

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 1;
  pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;

  if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr,
                             &m_pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout!");
  }

  GraphicsPipelineBuilder builder;
  m_pipeline =
      builder.AddShaderStage(VK_SHADER_STAGE_VERTEX_BIT, vertModule, "main")
          .AddShaderStage(VK_SHADER_STAGE_FRAGMENT_BIT, fragModule, "main")
          // No vertex input state, we use gl_VertexIndex and gl_InstanceIndex
          .SetVertexInput(nullptr, 0, nullptr, 0)
          .SetViewport(extent)
          .SetPipelineLayout(m_pipelineLayout)
          .UseDynamicRendering(vulkanCtx.GetSwapchain()->GetFormat().format,
                               VK_FORMAT_UNDEFINED)
          .EnableAlphaBlend()
          .Build();

  vkDestroyShaderModule(device, vertModule, nullptr);
  vkDestroyShaderModule(device, fragModule, nullptr);
}

void GsApp::SortSplats() {
  if (m_splatIndices.empty())
    return;

  glm::mat4 view = m_camera.GetViewMatrix();
  // Compute depth for all splats
  for (size_t i = 0; i < m_splatIndices.size(); ++i) {
    uint32_t initial_idx = m_splatIndices[i].index;
    const auto &pos = m_gpuSplats[initial_idx].position_opacity;
    glm::vec4 viewPos = view * glm::vec4(pos.x, pos.y, pos.z, 1.0f);
    m_splatIndices[i].depth = viewPos.z;
  }

  // Sort far to near (Vulkan view Z is negative into screen, so larger Z is
  // closer, meaning we sort ascending Z? Wait, GLM lookAt uses right-handed, so
  // forward is -Z. Far means more negative Z. We want to render back-to-front.
  // Back is more negative Z. So we sort ascending (smallest Z first).
  std::sort(std::execution::par_unseq, m_splatIndices.begin(),
            m_splatIndices.end(),
            [](const gs::SplatSortEntry &a, const gs::SplatSortEntry &b) {
              return a.depth < b.depth;
            });

  // Upload indices to SSBO
  if (m_indexBuffer) {
    void *data = m_indexBuffer->Map();
    // We only map the uint32_t indices. We can extract them or pack them
    // Actually, m_splatIndices is an array of structs (uint32_t, float).
    // Our SSBO expects an array of uints.
    uint32_t *mappedUints = reinterpret_cast<uint32_t *>(data);
    for (size_t i = 0; i < m_splatIndices.size(); ++i) {
      mappedUints[i] = m_splatIndices[i].index;
    }
    m_indexBuffer->Unmap();
  }
}

void GsApp::UpdateUniformBuffer() {
  CameraUBO ubo{};
  ubo.view = m_camera.GetViewMatrix();
  ubo.proj = m_camera.GetProjectionMatrix(m_width / m_height);
  ubo.viewport = glm::vec2(m_width, m_height);

  void *data = m_uniformBuffer->Map();
  memcpy(data, &ubo, sizeof(ubo));
  m_uniformBuffer->Unmap();
}

void GsApp::OnDrawFrame() {
  if (m_splatIndices.empty())
    return;

  SortSplats();
  UpdateUniformBuffer();

  auto &vulkanCtx = VulkanContext::Get();

  if (vulkanCtx.AcquireNextImage() != VK_SUCCESS) {
    return; // Swapchain recreated or failed
  }

  auto *frameCtx = vulkanCtx.GetCurrentFrameContext();
  auto &commandBuffer = frameCtx->commandBuffer;
  commandBuffer->Begin();

  auto &swapchain = vulkanCtx.GetSwapchain();
  auto imageView = swapchain->GetCurrentView();
  auto extent = swapchain->GetExtent();

  VkImageSubresourceRange range{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};

  commandBuffer->TransitionLayout(
      swapchain->GetCurrentImage(), range,
      ImageLayoutTransition::FromUndefinedToColorAttachment());

  VkRenderingAttachmentInfo colorAttachment{};
  colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
  colorAttachment.imageView = imageView;
  colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
  colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
  colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
  VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
  colorAttachment.clearValue = clearColor;

  VkRenderingInfo renderingInfo{};
  renderingInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
  renderingInfo.renderArea.offset = {0, 0};
  renderingInfo.renderArea.extent = extent;
  renderingInfo.layerCount = 1;
  renderingInfo.colorAttachmentCount = 1;
  renderingInfo.pColorAttachments = &colorAttachment;

  vkCmdBeginRendering(*commandBuffer, &renderingInfo);

  vkCmdBindPipeline(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                    m_pipeline);
  vkCmdBindDescriptorSets(*commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                          m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);

  // Draw 4 vertices (quad) for each splat
  vkCmdDraw(*commandBuffer, 4, static_cast<uint32_t>(m_splatIndices.size()), 0,
            0);

  vkCmdEndRendering(*commandBuffer);

  commandBuffer->TransitionLayout(swapchain->GetCurrentImage(), range,
                                  ImageLayoutTransition::FromColorToPresent());

  commandBuffer->End();

  vulkanCtx.SubmitPresent();
}

void GsApp::OnCleanup() {
  auto device = VulkanContext::Get().GetVkDevice();
  vkDeviceWaitIdle(device);

  if (m_pipeline) {
    vkDestroyPipeline(device, m_pipeline, nullptr);
    m_pipeline = VK_NULL_HANDLE;
  }
  if (m_pipelineLayout) {
    vkDestroyPipelineLayout(device, m_pipelineLayout, nullptr);
    m_pipelineLayout = VK_NULL_HANDLE;
  }
  if (m_descriptorSetLayout) {
    vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
    m_descriptorSetLayout = VK_NULL_HANDLE;
  }
  if (m_descriptorPool) {
    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);
    m_descriptorPool = VK_NULL_HANDLE;
  }

  if (m_splatBuffer) {
    m_splatBuffer->Cleanup();
    m_splatBuffer.reset();
  }
  if (m_indexBuffer) {
    m_indexBuffer->Cleanup();
    m_indexBuffer.reset();
  }
  if (m_uniformBuffer) {
    m_uniformBuffer->Cleanup();
    m_uniformBuffer.reset();
  }
}

void GsApp::ProcessInput(const Uint8 *state, float deltaTime) {
  m_camera.ProcessKeyboard(state, deltaTime);
}

void GsApp::ProcessMouseMotion(float xrel, float yrel) {
  m_camera.ProcessMouseMovement(xrel, -yrel); // Invert y
}

#if defined(__ANDROID__)
void GsApp::OnSurfaceChanged() {
  auto &vulkanCtx = VulkanContext::Get();
  vulkanCtx.RecreateSwapchain();
  auto extent = vulkanCtx.GetSwapchainExtent();
  m_width = (float)extent.width;
  m_height = (float)extent.height;
}
#endif
