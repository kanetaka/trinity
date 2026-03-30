#pragma once
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>
#include <algorithm>
#include <cassert>

namespace tri
{
    using Entity = uint32_t;
    const Entity NullEntity = 0;

    class IComponentPool
    {
    public:
        virtual ~IComponentPool() = default;
        virtual void Remove(Entity entity) = 0;
    };

    template<typename T>
    class ComponentPool : public IComponentPool
    {
    public:
        T& Add(Entity entity, T&& component)
        {
            if (entity_to_index_.find(entity) != entity_to_index_.end())
            {
                return components_[entity_to_index_[entity]];
            }

            uint32_t index = static_cast<uint32_t>(components_.size());
            entity_to_index_[entity] = index;
            index_to_entity_[index] = entity;
            components_.push_back(std::move(component));
            return components_.back();
        }

        void Remove(Entity entity) override
        {
            auto it = entity_to_index_.find(entity);
            if (it == entity_to_index_.end()) return;

            uint32_t index_to_remove = it->second;
            uint32_t last_index = static_cast<uint32_t>(components_.size() - 1);

            if (index_to_remove != last_index)
            {
                // Swap with last
                Entity last_entity = index_to_entity_[last_index];
                components_[index_to_remove] = std::move(components_[last_index]);
                entity_to_index_[last_entity] = index_to_remove;
                index_to_entity_[index_to_remove] = last_entity;
            }

            components_.pop_back();
            entity_to_index_.erase(entity);
            index_to_entity_.erase(last_index);
        }

        T* Get(Entity entity)
        {
            auto it = entity_to_index_.find(entity);
            if (it == entity_to_index_.end()) return nullptr;
            return &components_[it->second];
        }

        Entity GetEntityAtIndex(uint32_t index) const
        {
            auto it = index_to_entity_.find(index);
            return it != index_to_entity_.end() ? it->second : NullEntity;
        }

        bool Has(Entity entity) const
        {
            return entity_to_index_.find(entity) != entity_to_index_.end();
        }

        std::vector<T>& GetComponents() { return components_; }
        const std::vector<T>& GetComponents() const { return components_; }

        uint32_t GetIndex(Entity entity) const
        {
            auto it = entity_to_index_.find(entity);
            return it != entity_to_index_.end() ? it->second : 0xFFFFFFFF;
        }

    private:
        std::vector<T> components_;
        std::unordered_map<Entity, uint32_t> entity_to_index_;
        std::unordered_map<uint32_t, Entity> index_to_entity_;
    };

    class Registry
    {
    public:
        Entity Create()
        {
            return ++next_entity_id_;
        }

        void Destroy(Entity entity)
        {
            for (auto& pair : pools_)
            {
                pair.second->Remove(entity);
            }
        }

        template<typename T>
        T& AddComponent(Entity entity, T&& component = T())
        {
            return GetPool<T>()->Add(entity, std::move(component));
        }

        template<typename T>
        void RemoveComponent(Entity entity)
        {
            GetPool<T>()->Remove(entity);
        }

        template<typename T>
        T* GetComponent(Entity entity)
        {
            return GetPool<T>()->Get(entity);
        }

        template<typename T>
        bool HasComponent(Entity entity)
        {
            return GetPool<T>()->Has(entity);
        }

        template<typename T>
        std::vector<T>& View()
        {
            return GetPool<T>()->GetComponents();
        }

        template<typename T>
        uint32_t GetPoolIndex(Entity entity)
        {
            return GetPool<T>()->GetIndex(entity);
        }

        template<typename T, typename Func>
        void ForEach(Func&& func)
        {
            auto* pool = GetPool<T>();
            auto& components = pool->GetComponents();
            for (size_t i = 0; i < components.size(); ++i)
            {
                Entity entity = pool->GetEntityAtIndex(static_cast<uint32_t>(i));
                func(entity, components[i]);
            }
        }

    private:
        template<typename T>
        ComponentPool<T>* GetPool()
        {
            uint32_t type_id = GetTypeId<T>();
            if (pools_.find(type_id) == pools_.end())
            {
                pools_[type_id] = std::make_unique<ComponentPool<T>>();
            }
            return static_cast<ComponentPool<T>*>(pools_[type_id].get());
        }

        template<typename T>
        uint32_t GetTypeId()
        {
            static uint32_t type_id = next_type_id_++;
            return type_id;
        }

        Entity next_entity_id_ = 0;
        inline static uint32_t next_type_id_ = 0;
        std::unordered_map<uint32_t, std::unique_ptr<IComponentPool>> pools_;
    };
} // namespace tri
