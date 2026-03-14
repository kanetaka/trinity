#pragma once
#include "core/vulkan_context.h"


// Vulkan の構造体 pNext を繋ぐ処理簡略化のためのテンプレート
template<typename T>
void BuildVkExtensionChain(T& last)
{
    last.pNext = nullptr;
}
template<typename T, typename U, typename ... Rest>
void BuildVkExtensionChain(T& current, U& next, Rest&... rest)
{
    current.pNext = &next;
    BuildVkExtensionChain(next, rest...);
}

class IExtensionFeatureProvider
{
public:
    virtual ~IExtensionFeatureProvider() = default;

    // 有効にするインスタンス拡張機能の名前リストを返す
    virtual void GetRequiredInstanceExtensions(std::vector<const char*>& extensionList) = 0;

    // 有効にするデバイス拡張機能の名前リストを返す
    virtual void GetRequiredDeviceExtensions(std::vector<const char*>& extensionList) = 0;

    // pNext で繋ぐためのポインタを返す
    void* GetDeviceFeatures() {
        return reinterpret_cast<void*>(&phys_dev_features_);
    }

    // m_physDevFeaturesからの拡張機能のリンクを構成する
    virtual void BuildFeatures(VulkanContext* vulkanCtx) = 0;

protected:
    VkPhysicalDeviceFeatures2 phys_dev_features_{ .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
};
