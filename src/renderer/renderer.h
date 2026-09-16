#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>

namespace tri
{
    class Application;
    class Object;
    class Scene;
    class IComponent;
    class IRenderable;
}

namespace tri
{
    class CommandBuffer;
    class UniformBuffer;
    class StorageBuffer;

    struct RenderItem
    {
        IRenderable* renderable;
        uint32_t transform_index;
        int render_order;
    };

    class Renderer
    {
    public:
        Renderer(Application* app);
        ~Renderer();

        bool Initialize(float screen_width, float screen_height);
        void Shutdown();

        void Draw(Object& root);

        void SetViewMatrix(const glm::mat4& view) { view_ = view; }
        void SetProjectionMatrix(const glm::mat4& proj) { projection_ = proj; }
        void SetCameraPosition(const glm::dvec3& pos) { camera_pos_ = pos; }

        void UpdateUniformBuffer();
        void UpdateTransformBuffer(const Scene& scene);

        void RegisterComponent(IComponent& component, int render_order = 0);

        template<typename T>
        void SetDefaultOrder(int order)
        {
            type_orders_[std::type_index(typeid(T))] = order;
        }

        VkDescriptorSetLayout GetDescriptorSetLayout() const { return descriptor_set_layout_; }
        VkDescriptorSet AllocateDescriptorSet();

        float GetScreenWidth() const { return screen_width_; }
        float GetScreenHeight() const { return screen_height_; }

    private:
        bool CreateDescriptorSetLayout();
        bool CreateDescriptorPool();
        bool CreateDescriptorSets();
        bool InitializeGraphicsPipeline();
        void CollectRenderItems(Object& object, std::vector<RenderItem>& out_items);

        Application* app_;

        VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
        VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;

        VkPipeline pipeline_ = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;

        std::shared_ptr<UniformBuffer> uniform_buffer_;
        std::shared_ptr<StorageBuffer> transform_buffer_;

        std::unordered_map<std::type_index, int> type_orders_;

        glm::mat4 view_;
        glm::mat4 projection_;
        glm::dvec3 camera_pos_;
        float screen_width_;
        float screen_height_;
    };


} // namespace tri
