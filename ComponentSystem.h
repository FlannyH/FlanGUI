#pragma once
#include <vector>

#define MAX_ENTITIES 1024

namespace Flan {
    struct Pool {
        uint8_t* pool = nullptr;
        size_t comp_size = 0;

        Pool() = default;

        explicit Pool(const size_t comp_size_) {
            comp_size = comp_size_;
            pool = new uint8_t[MAX_ENTITIES * comp_size];
            printf("create pool: %p\n", pool);
        }

        [[nodiscard]] void* get(const size_t index) const {
            return pool + index * comp_size;
        }
    };

    inline int comp_ctr = 0;

    template <class T>
    uint64_t get_comp_id()
    {
        static uint64_t comp_id = comp_ctr++;
        return comp_id;
    }

    typedef size_t EntityID;

    class Scene {
    public:
        EntityID new_entity();
        // Add a component from an entity, initializing the component with the values specified in the argument
        template <typename T>
        void add_component(EntityID entity, T comp);

        // Add a component from an entity, initializing the component using its default constructor
        template <typename T>
        void add_component(EntityID entity);

        // Remove a component from an entity
        template <class T>
        void remove_compoment(EntityID entity);

        // Get a pointer to this entity's specified component. If the entity does not have the specified component, nullptr is returned.
        template <class T>
        T* get_component(EntityID entity);

        // Get a view of all the components with the given components
        template <typename t1, typename t2 = void, typename t3 = void, typename t4 = void>
        std::vector<EntityID> view();
    private:
        std::vector<Pool> _pools;
        std::vector<uint64_t> _entities;
    };

}

namespace Flan {
    template <typename T>
    void Scene::add_component(EntityID entity, T comp) {
        auto comp_id = get_comp_id<T>();
        // Set the component flag for this component
        _entities[entity] |= 1 << comp_id;

        // If the pool for this component does not exist, create one
        if (comp_id >= _pools.size()) {
            _pools.resize(comp_id + 1);
            _pools[comp_id] = { sizeof(T) };
        }

        // Initialize the component
        memcpy_s(_pools[comp_id].get(entity), sizeof(T), &comp, sizeof(T));
    }

    template <typename T>
    void Scene::add_component(EntityID entity) {
        auto comp_id = get_comp_id<T>();
        // Set the component flag for this component
        _entities[entity] |= 1 << comp_id;

        // If the pool for this component does not exist, create one
        if (comp_id >= _pools.size()) {
            _pools.resize(comp_id + 1);
            _pools[comp_id] = Pool(sizeof(T));
        }

        // Initialize the component
        new (_pools[comp_id].get(entity)) T();
    }

    template <class T>
    void Scene::remove_compoment(EntityID entity) {
        // Reset the component flag for this component
        _entities[entity] &= ~(1 << get_comp_id<T>());
    }

    template <class T>
    T* Scene::get_component(EntityID entity) {
        // If the entity has this component
        if (_entities[entity] & (1 << get_comp_id<T>())) {
            // Return the component
            return static_cast<T*>(_pools[get_comp_id<T>()].get(entity));
        }

        // Otherwise return null
        return nullptr;
    }

    template <typename t1, typename t2, typename t3, typename t4>
    std::vector<EntityID> Scene::view() {
        std::vector<EntityID> out;
        for (EntityID i = 0; i < _entities.size(); i++) {
            // If the first component isn't present, skip this entity
            if ((_entities[i] & (1ull << get_comp_id<t1>())) == 0) {
                continue;
            }
            // If the second component isn't present, skip this entity
            if ((_entities[i] & (1ull << get_comp_id<t2>())) == 0 && std::is_same_v<t2, void> == false) {
                continue;
            }
            // If the third component isn't present, skip this entity
            if ((_entities[i] & (1ull << get_comp_id<t3>())) == 0 && std::is_same_v<t3, void> == false) {
                continue;
            }
            // If the fourth component isn't present, skip this entity
            if ((_entities[i] & (1ull << get_comp_id<t4>())) == 0 && std::is_same_v<t4, void> == false) {
                continue;
            }
            // Otherwise, add it to the view
            out.push_back(i);
        }
        return out;
    }

    inline EntityID Scene::new_entity() {
        // Is there an empty spot in the entities list? If so, claim that one
        for (EntityID i = 0; i < _entities.size(); i++) {
            if (_entities[i] == 0) {
                _entities[i] = 1ull << ((sizeof(EntityID) * 8ull) - 1ull); // enable flag is the most significant bit
                return i;
            }
        }

        // Otherwise, expand the list
        if (_entities.size() < MAX_ENTITIES) {
            _entities.push_back(1ull << ((sizeof(EntityID) * 8ull) - 1ull));
            return static_cast<EntityID>(_entities.size()) - 1;
        }

        // If this fails, there's a problem
        printf("ERROR: ran out of entity slots!\n");
        throw;
    }
}