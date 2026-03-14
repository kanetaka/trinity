#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <memory>
#include <functional>
#include <stdint.h>
#include <string>
#include <cstring>

#include "core/command_buffer.h"

class Swapchain;
class CommandBuffer;
class ISurfaceProvider;

class VulkanContext {
public:
        static constexpr uint32_t MaxInflightFrames = 2;
        static VulkanContext& Get();

        // 初期化
        void Initialize(const char* appName, ISurfaceProvider* surfaceProvider);

        // 終了処理
        void Cleanup();

        // スワップチェインの作成
        void RecreateSwapchain();

        // 各種Vulkanオブジェクト取得
        VkInstance GetVkInstance() const { return vk_instance_; }
        VkDevice   GetVkDevice() const { return vk_device_; }
        VkPhysicalDevice GetVkPhysicalDevice() const { return vk_physical_device_; }
        VkDescriptorPool GetVkDescriptorPool() const { return descriptor_pool_; }

        VkQueue GetGraphicsQueue() const    { return graphics_queue_; }
        uint32_t GetGraphicsFamily() const  { return graphics_queue_family_index_; }
        uint32_t GetPresentFamily() const  { return present_queue_family_index_; }

        VkCommandPool GetCommandPool() const { return command_pool_; }
        VkSurfaceKHR GetSurface() const     { return surface_; }

        // コマンドバッファの作成
        std::shared_ptr<CommandBuffer> CreateCommandBuffer();

        // ディスクリプタセットの確保
        VkDescriptorSet AllocateDescriptorSet(VkDescriptorSetLayout layout);
        // ディスクリプタセットの解放
        void FreeDescriptorSet(VkDescriptorSet descriptorSet);

        // 描画フレーム単位で取り扱うコンテキスト情報
        struct FrameContext
        {
                std::shared_ptr<CommandBuffer> commandBuffer;
                VkFence     inflightFence = VK_NULL_HANDLE;
        };
        // 現在のフレームインデックスを取得
        uint32_t GetCurrentFrameIndex() const { return current_frame_index_; }
        // 描画可能なスワップチェインイメージの切り替え
        VkResult AcquireNextImage();

        // 現在のフレームコンテキストのコマンドを実行し、プレゼンテーションを発行
        void SubmitPresent();

        // 指定されたコマンドバッファを実行し、完了を待機
        void SubmitAndWait(std::shared_ptr<CommandBuffer> commandBuffer);

        // 現在フレームコンテキストの取得
        FrameContext* GetCurrentFrameContext();

        // スワップチェインの取得
        std::unique_ptr<Swapchain>& GetSwapchain() { return swapchain_; }

        // メモリタイプの取得
        uint32_t FindMemoryType(const VkMemoryRequirements& requirements, VkMemoryPropertyFlags properties) const;

        // ユニフォームバッファオフセットのアライメント制約
        uint32_t MinUniformOffsetAlignment() const;

        // ストレージバッファオフセットのアライメント制約
        uint32_t MinStorageBufferOffsetAlignment() const;

        // CPU-GPUの間で非コヒーレントなメモリにおける最小の同期単位
        uint32_t NonCoherentAtomSize() const;

        // Function Callback(s)
        std::function<void(std::vector<const char*>&)> GetWindowSystemExtensions;

        // オブジェクトに名前を設定する
        void SetDebugObjectName(void* objectHandle, VkObjectType type, const char* name);

private:
        VulkanContext() = default;
        ~VulkanContext() = default;
private:
        void CreateInstance(const char* appName);
        void CreateSurface();
        void PickPhysicalDevice();
        void CreateLogicalDevice();
        void CreateDebugMessenger();
        void CreateCommandPool();
        void CreateDescriptorPool();

        void CreateFrameContexts();
        void DestroyFrameContexts();

        void AdvanceFrame();
        void BuildVkFeatures();

        ISurfaceProvider* surface_provider_{};
        VkInstance      vk_instance_{};

        VkPhysicalDevice vk_physical_device_{};
        VkDevice        vk_device_{};
        VkQueue         graphics_queue_{};
        uint32_t        graphics_queue_family_index_{};
        uint32_t        present_queue_family_index_{};
        VkPhysicalDeviceMemoryProperties memory_properties_{};
        VkPhysicalDeviceProperties physical_device_properties_{};

        VkSurfaceKHR    surface_{};
        VkCommandPool   command_pool_{};
        VkDescriptorPool descriptor_pool_{};
        std::vector<FrameContext> frame_context_;
        std::unique_ptr<Swapchain> swapchain_;

        VkDebugUtilsMessengerEXT debug_messenger_{};
        PFN_vkSetDebugUtilsObjectNameEXT pfn_set_debug_utils_object_name_ext_{};

        uint32_t current_frame_index_ = 0;

        // --------
        VkPhysicalDeviceFeatures2 phys_dev_features_{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2
        };
        VkPhysicalDeviceVulkan11Features vulkan11_features_{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES
        };
        VkPhysicalDeviceVulkan12Features vulkan12_features_{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES
        };
        VkPhysicalDeviceVulkan13Features vulkan13_features_{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES
        };
        VkPhysicalDeviceShaderAtomicFloatFeaturesEXT atomic_float_features_
        {
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_ATOMIC_FLOAT_FEATURES_EXT
        };
};
