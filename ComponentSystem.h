#pragma once
#include <vector>
#include <cassert>

#define MAX_ENTITIES 1024

namespace Flan {
    struct Pool {
        uint8_t* pool = nullptr;
        size_t comp_size = 0;

        Pool() = default;
        ~Pool() {
            free(pool);
        }

        void init(size_t comp_size_) {
            comp_size = comp_size_;
            free(pool);
            pool = static_cast<uint8_t*>(malloc(MAX_ENTITIES * comp_size));
        }

        explicit Pool(const size_t comp_size_) {
            init(comp_size_);
        }

        [[nodiscard]] void* get(const size_t index) const {
            assert(pool != nullptr);
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

    using EntityID = size_t;

    class SceneViewIterator {
    public:
        SceneViewIterator(EntityID*& set_entities, size_t set_index) : entities(set_entities), index(set_index) {}
        SceneViewIterator& operator++() {
            ++index;
            return *this;
        }

        EntityID& operator*() {
            return entities[index];
        }

        bool operator==(const SceneViewIterator& other) {
            return entities == other.entities && index == other.index;
        }

        bool operator!=(const SceneViewIterator& other) {
            return !(*this == other);
        }

    private:
        EntityID*& entities;
        size_t index;
    };

    struct SceneView {
        EntityID* entities;
        size_t n_entities;
        SceneViewIterator begin() {
            return SceneViewIterator(entities, 0);
        }
        SceneViewIterator end() {
            return SceneViewIterator(entities, n_entities);
        }
    };

    class Scene {
    public:
        Scene() {
            view_out = new EntityID[8192];
        }
        EntityID new_entity();
        // Add a component from an entity, initializing the component by copying an existing object
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
        SceneView view();
    private:
        std::vector<Pool> _pools = std::vector<Pool>(64);
        std::vector<uint64_t> _entities;
        EntityID* view_out;
    };

}

namespace Flan {
    template <typename T>
    void Scene::add_component(EntityID entity, T comp) {
        auto comp_id = get_comp_id<T>();
        // Set the component flag for this component
        _entities[entity] |= 1ull << comp_id;

        // If the pool for this component does not exist, create one
        if (comp_id >= _pools.size()) {
            _pools.resize(comp_id + 1);
            _pools[comp_id].init(sizeof(T));
        }

        // If the pool is null, initialize it
        if (_pools[comp_id].pool == nullptr) {
            _pools[comp_id].init(sizeof(T));
        }

        // Initialize the component
        new (_pools[comp_id].get(entity)) T(comp);
    }

    template <typename T>
    void Scene::add_component(EntityID entity) {
        uint64_t comp_id = get_comp_id<T>();
        // Set the component flag for this component
        _entities[entity] |= 1ull << comp_id;

        // If the pool for this component does not exist, create one
        if (comp_id >= _pools.size()) {
            _pools.resize(comp_id + 1);
            _pools[comp_id] = Pool(sizeof(T));
        }

        // If the pool is null, initialize it
        if (_pools[comp_id].pool == nullptr) {
            _pools[comp_id].init(sizeof(T));
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
        if (_entities[entity] & (1ull << get_comp_id<T>())) {
            // Return the component
            return static_cast<T*>(_pools[get_comp_id<T>()].get(entity));
        }

        // Otherwise return null
        return nullptr;
    }

    template <typename t1, typename t2, typename t3, typename t4>
    SceneView Scene::view() {
        size_t entity_id = 0;
        for (EntityID i = 0; i < _entities.size(); i++) {
            // If the first component isn't present, skip this entity
            if ((_entities[i] & (1ull << get_comp_id<t1>())) == 0) {
                continue;
            }
            // If the second component isn't present, skip this entity
            if ((_entities[i] & (1ull << get_comp_id<t2>())) == 0 && !std::is_same_v<t2, void>) {
                continue;
            }
            // If the third component isn't present, skip this entity
            if ((_entities[i] & (1ull << get_comp_id<t3>())) == 0 && !std::is_same_v<t3, void>) {
                continue;
            }
            // If the fourth component isn't present, skip this entity
            if ((_entities[i] & (1ull << get_comp_id<t4>())) == 0 && !std::is_same_v<t4, void>) {
                continue;
            }
            // Otherwise, add it to the view
            view_out[entity_id++] = i;
        }
        return { view_out, entity_id };
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