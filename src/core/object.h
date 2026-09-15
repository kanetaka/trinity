#pragma once
#include <algorithm>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include "core/component.h"

namespace tri
{
    constexpr uint32_t kInvalidTransformIndex = 0xFFFFFFFF;

    // Node of the scene graph. Behavior is attached through components implementing
    // IComponent (and further interfaces such as IRenderable), never through subclassing Object.
    class Object
    {
    public:
        ~Object();

        Object(const Object&) = delete;
        Object& operator=(const Object&) = delete;

        const std::string& GetName() const { return name_; }

        Object* GetParent() const { return parent_; }
        const std::vector<std::unique_ptr<Object>>& GetChildren() const { return children_; }

        void SetLocalPosition(const glm::dvec3& position);
        void SetLocalRotation(const glm::quat& rotation);
        void SetLocalScale(float scale);
        const glm::dvec3& GetLocalPosition() const { return local_position_; }
        const glm::quat& GetLocalRotation() const { return local_rotation_; }
        float GetLocalScale() const { return local_scale_; }

        const glm::dmat4& GetWorldTransform() const { return world_transform_; }
        void UpdateWorldTransform(const glm::dmat4& parent_world);

        uint32_t GetTransformIndex() const { return transform_index_; }

        template<typename T, typename... Args>
        T& AddComponent(Args&&... args)
        {
            static_assert(std::is_base_of_v<IComponent, T>, "T must implement IComponent");
            auto component = std::make_unique<T>(std::forward<Args>(args)...);
            T& ref = *component;
            components_.push_back(std::move(component));
            return ref;
        }

        template<typename T>
        T* GetComponent() const
        {
            for (auto& component : components_)
            {
                if (auto* casted = dynamic_cast<T*>(component.get()))
                {
                    return casted;
                }
            }
            return nullptr;
        }

        template<typename T>
        void RemoveComponent()
        {
            components_.erase(
                std::remove_if(components_.begin(), components_.end(),
                    [](const std::unique_ptr<IComponent>& component) { return dynamic_cast<T*>(component.get()) != nullptr; }),
                components_.end());
        }

        // Invokes func for every attached component implementing Interface.
        template<typename Interface, typename Func>
        void ForEachComponent(Func&& func) const
        {
            for (auto& component : components_)
            {
                if (auto* casted = dynamic_cast<Interface*>(component.get()))
                {
                    func(*casted);
                }
            }
        }

    private:
        friend class World;
        explicit Object(std::string name);

        std::string name_;
        Object* parent_ = nullptr;
        std::vector<std::unique_ptr<Object>> children_;
        std::vector<std::unique_ptr<IComponent>> components_;

        glm::dvec3 local_position_ = glm::dvec3(0.0);
        glm::quat local_rotation_ = glm::identity<glm::quat>();
        float local_scale_ = 1.0f;
        bool transform_dirty_ = true;
        glm::dmat4 local_transform_ = glm::dmat4(1.0);
        glm::dmat4 world_transform_ = glm::dmat4(1.0);

        uint32_t transform_index_ = kInvalidTransformIndex;
    };
} // namespace tri
