#pragma once
#include <cstdint>
#include <memory>
#include <vulkan/vulkan.h>

class Entity;
class CommandBuffer;

class Component
{
public:
    Component(Entity* owner, int update_order = 100);
    virtual ~Component();

    virtual void Update(float delta_time);
    virtual void ProcessInput(const uint8_t* key_state) {}
    virtual void OnUpdateWorldTransform() {}
    virtual void Draw(std::shared_ptr<CommandBuffer>& command_buffer, VkPipelineLayout pipeline_layout) {}

    int GetUpdateOrder() const { return update_order_; }

protected:
    Entity* owner_;
    int update_order_;
};
