#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <functional>
#ifndef GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#endif
#include <glm/glm.hpp>

#include <cassert>

namespace tri
{
    class Camera;
    class GraphicsContext;
    class Object;
    class Scene;
    class IComponent;
    class IRenderable;
    class UiManager;
    class UniformBuffer;
    class StorageBuffer;

    struct RenderItem
    {
        IRenderable* renderable;
        const Object* object;
        int render_order;
    };

    class Renderer
    {
    public:
        Renderer(GraphicsContext& context, Scene* scene = nullptr);
        ~Renderer();

        bool Initialize(float screen_width = 0.0f, float screen_height = 0.0f);
        void Shutdown();
        void WaitIdle();

        void SetScene(Scene* scene)
        {
            scene_ = scene;
            render_items_dirty_ = true;
        }
        Scene* GetScene() const { return scene_; }

        void SetCamera(const Camera* camera) { camera_ = camera; }
        const Camera* GetCamera() const { return camera_; }

        void UpdateCamera(float aspect);
        void UpdateCamera(const Camera& camera, float aspect);

        void PreRender();
        void PreRender(const Camera& camera);
        void PreRender(Scene& scene, const Camera& camera);

        void Draw();
        void Draw(UiManager* ui_manager);
        void Draw(const Scene& scene, UiManager* ui_manager = nullptr);
        void Draw(Object& root, UiManager* ui_manager = nullptr);

        void SetUiManager(UiManager* ui_manager) { ui_manager_ = ui_manager; }
        UiManager* GetUiManager() const { return ui_manager_; }

        void SetViewMatrix(const glm::mat4& view) { view_ = view; }
        void SetProjectionMatrix(const glm::mat4& proj) { projection_ = proj; }
        void SetCameraPosition(const glm::dvec3& pos) { camera_pos_ = pos; }

        void UpdateUniformBuffer();
        void UpdateTransformBuffer();
        void UpdateTransformBuffer(const Scene& scene);

        void RegisterComponent(IComponent& component, int render_order = 0);

        template<typename T>
        void SetDefaultOrder(int order)
        {
            type_orders_[std::type_index(typeid(T))] = order;
            render_items_dirty_ = true;
        }

        void MarkRenderDirty() { render_items_dirty_ = true; }

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
        void RenderInternal(UiManager* ui_manager);

        GraphicsContext& context_;

        VkDescriptorSetLayout descriptor_set_layout_ = VK_NULL_HANDLE;
        VkDescriptorPool descriptor_pool_ = VK_NULL_HANDLE;

        VkPipeline pipeline_ = VK_NULL_HANDLE;
        VkPipelineLayout pipeline_layout_ = VK_NULL_HANDLE;

        std::shared_ptr<UniformBuffer> uniform_buffer_;
        std::shared_ptr<StorageBuffer> transform_buffer_;

        std::unordered_map<std::type_index, int> type_orders_;

        Scene* scene_ = nullptr;
        const Camera* camera_ = nullptr;
        UiManager* ui_manager_ = nullptr;
        std::vector<RenderItem> render_items_;
        bool render_items_dirty_ = true;

        glm::mat4 view_;
        glm::mat4 projection_;
        glm::dvec3 camera_pos_;
        float screen_width_;
        float screen_height_;
    };
} // namespace tri
