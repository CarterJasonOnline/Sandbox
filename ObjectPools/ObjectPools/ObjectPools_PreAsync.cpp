// ObjectPools.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ObjectPool.h"
#include "ObjectWithData.h"

int main() {

    size_t totalObjects = 5;
    const size_t totalTests = 10;

    // Create an Object Pool
    ObjectPool<ObjectWithData> pool(totalObjects);

    // Run the test defined times
    for (auto test = 0; test < totalTests; ++test) {

        std::cout << "Test: " << test << std::endl;

        // Allocate the Objects Buffers
        std::vector<std::unique_ptr<ObjectWithData>> objects;
        for (auto i = 0; i < totalObjects; ++i) {
            auto obj = pool.acquire();
            obj->allocateBuffer(std::rand() % 1024);
            objects.push_back(std::move(obj));
        }

        // Release the Objects back to the pool
        for (auto& object : objects) {
            pool.release(std::move(object));
        }

        // Increase the total objects by 2 - Grow the pool
        totalObjects *= 2;
    }

    return 0;
}

