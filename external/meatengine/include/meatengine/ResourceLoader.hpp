#pragma once

#include <entt/resource/resource.hpp>
#include <entt/core/hashed_string.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <utility>

namespace me {
    class ResourceLoader {
    private:
        template<typename T>
        struct Cache {
            std::unordered_map<entt::id_type, std::shared_ptr<T>> map;
        };

        template<typename T>
        static Cache<T>& get_cache() {
            static Cache<T> cache;
            return cache;
        }

    public:
        template<typename T>
        static std::shared_ptr<T>& default_handle() {
            static std::shared_ptr<T> h;
            return h;
        }

        template<typename T>
        static void set_default(const entt::resource<T>& res) {
            default_handle<T>() = res.handle();
        }

        template<typename T>
        static std::shared_ptr<T> get_default() {
            return default_handle<T>();
        }

        template<typename T>
        static bool has_default() {
            return static_cast<bool>(default_handle<T>());
        }
        
        template<typename T, typename... Args>
        static entt::resource<T> load(Args&&... args) {
            auto& cache = get_cache<T>();

            std::string id_str;
            ((id_str += std::string{args} + "|"), ...);
            if (!id_str.empty()) id_str.pop_back();

            if (id_str.empty()) return entt::resource<T>{nullptr};
            
            auto id = entt::hashed_string{id_str.c_str()};

            auto it = cache.map.find(id);
            if (it != cache.map.end()) {
                return entt::resource<T>{it->second};
            }

            auto resource = T{}(std::forward<Args>(args)...);
            if (!resource) {
                return entt::resource<T>{nullptr};
            }

            cache.map[id] = resource;
            return entt::resource<T>{resource};
        }

        template<typename T, typename... Args>
        static entt::resource<T> get(Args&&... args) {
            auto& cache = get_cache<T>();

            std::string id_str;
            ((id_str += std::string{std::forward<Args>(args)} + "|"), ...);
            if (!id_str.empty()) id_str.pop_back();

            if (id_str.empty()) return entt::resource<T>{nullptr};

            auto id = entt::hashed_string{id_str.c_str()};

            auto it = cache.map.find(id);
            if (it != cache.map.end()) {
                return entt::resource<T>{it->second};
            }
            return entt::resource<T>{nullptr};
        }

        template<typename T>
        static std::shared_ptr<T> find_handle(const T* raw) {
            if (!raw) return nullptr;
            auto& cache = get_cache<T>();
            for (auto& [id, ptr] : cache.map) {
                if (ptr.get() == raw) return ptr;
            }
            return nullptr;
        }
    };
}