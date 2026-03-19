#pragma once
#include "core/graphics/vulkan_context.h"
#include "core/graphics/resources/gpu_resource_base.h"

class IImageResource
{
public:
	virtual VkFormat GetFormat() const = 0;
	virtual VkExtent2D GetExtent() const = 0;
	virtual uint32_t GetMipmapCount() const = 0;

	virtual VkImage GetVkImage() const = 0;
	virtual void SetAccessFlag(const VkAccessFlags flags) = 0;
	virtual VkAccessFlags GetAccessFlags() const = 0;

	virtual void SetLayout(VkImageLayout layout) = 0;
	virtual VkImageLayout GetLayout() const = 0;
};

template<typename T>
class ImageResource : public GpuResourceBase<T>, public IImageResource
{
public:
	ImageResource(const ImageResource&) = delete;
	ImageResource& operator=(const ImageResource&) = delete;

	virtual ~ImageResource() = default;
	virtual void Cleanup() = 0;
	virtual VkFormat GetFormat() const override { return format_; }
	virtual VkExtent2D GetExtent() const override { return extent_; }
	virtual uint32_t GetMipmapCount() const override { return mip_levels_; }

	virtual VkImage GetVkImage() const override { return image_; }
	virtual void SetAccessFlag(const VkAccessFlags flags) override { access_flags_ = flags; }
	virtual VkAccessFlags GetAccessFlags() const override { return access_flags_; }

	virtual void SetLayout(VkImageLayout layout) override { layout_ = layout; }
	virtual VkImageLayout GetLayout() const override { return layout_; }

protected:
	ImageResource() = default;

	VkImage image_ = VK_NULL_HANDLE;
	VkDeviceMemory memory_ = VK_NULL_HANDLE;
	VkImageSubresourceRange subresource_range_{};
	VkAccessFlags access_flags_ = VK_ACCESS_NONE;
	VkImageLayout layout_ = VK_IMAGE_LAYOUT_UNDEFINED;

	VkFormat format_ = VK_FORMAT_UNDEFINED;
	VkExtent2D extent_{};
	uint32_t mip_levels_{};
};

class DepthBuffer : public ImageResource<DepthBuffer>
{
	friend class GpuResourceBase<DepthBuffer>;
public:
	virtual ~DepthBuffer() { Cleanup(); }
	virtual void Cleanup() override;

	bool Initialize(VkExtent2D extent, VkFormat depthFormat);

	VkImageView GetVkImageView() const { return image_view_; }

	// Static factory method for creation and initialization
	static std::shared_ptr<DepthBuffer> Create(VkExtent2D extent, VkFormat depthFormat)
	{
		auto image = GpuResourceBase::Create();
		if (!image->Initialize(extent, depthFormat)) { return nullptr; }
		return image;
	}
private:
	VkImageView image_view_{};
};

class Texture2D : public ImageResource<Texture2D>
{
	friend class GpuResourceBase<Texture2D>;
public:
	virtual ~Texture2D() { Cleanup(); }
	virtual void Cleanup() override;

	bool Initialize(VkExtent2D extent, VkFormat format, uint32_t mipLevels);

	VkImageView GetVkImageView() const { return image_view_; }
	VkImageSubresourceRange GetSubresourceRange() const { return subresource_range_; }

	VkDescriptorImageInfo GetDescriptorInfo(VkSampler sampler) const;

	// Static factory method for creation and initialization
	static std::shared_ptr<Texture2D> Create(VkExtent2D extent, VkFormat format, uint32_t mipLevels)
	{
		auto image = GpuResourceBase::Create();
		if (!image->Initialize(extent, format, mipLevels)) { return nullptr; }
		return image;
	}

private:
	VkImageView image_view_{};

	VkImageSubresourceRange subresource_range_{};
};

class StorageImage2D : public ImageResource<StorageImage2D>
{
	friend class GpuResourceBase<StorageImage2D>;
public:
	virtual ~StorageImage2D() { Cleanup(); }
	virtual void Cleanup() override;

	bool Initialize(VkExtent2D extent, VkFormat format, uint32_t mipLevels);

	VkImageView GetVkImageView() const { return image_view_; }
	VkImageSubresourceRange GetSubresourceRange() const { return subresource_range_; }

	VkDescriptorImageInfo GetTextureReadDescriptorInfo(VkSampler sampler) const;
	VkDescriptorImageInfo GetStorageReadWriteDescriptorInfo(VkSampler sampler) const;

	// Static factory method for creation and initialization
	static std::shared_ptr<StorageImage2D> Create(VkExtent2D extent, VkFormat format, uint32_t mipLevels)
	{
		auto image = GpuResourceBase::Create();
		if (!image->Initialize(extent, format, mipLevels)) { return nullptr; }
		return image;
	}
private:
	VkImageView image_view_{};

	VkImageSubresourceRange subresource_range_{};
};
