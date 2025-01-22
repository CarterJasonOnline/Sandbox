#pragma once

#include <memory>
#include <vector>
#include <mutex>

template <typename T>
class ObjectPool {
public:
    using ObjectType = T;

    // Pre-allocate objects for the pool
    explicit ObjectPool(size_t poolSize) {
        for (size_t i = 0; i < poolSize; ++i) {
            pool.push_back(std::make_unique<ObjectType>(++idCounter));
        }
    }

    // Acquire an object from the pool
    template <typename... Args>
    std::unique_ptr<ObjectType> acquire(Args&&... args) {
        std::lock_guard<std::mutex> lock(poolMutex);
        if (pool.empty()) {
            return std::make_unique<ObjectType>(++idCounter);
        }
        else {
            auto obj = std::move(pool.back());
            pool.pop_back();
            return obj;
        }
    }

    // Release an object back to the pool
    void release(std::unique_ptr<ObjectType> obj) {
        std::lock_guard<std::mutex> lock(poolMutex);
        pool.push_back(std::move(obj));
    }

private:
    std::vector<std::unique_ptr<ObjectType>> pool;  // Underlying container to hold pooled objects
    std::mutex poolMutex;  // Lock for thread safety
    size_t idCounter = 0;  // Counter to assign unique IDs to objects
};