// ObjectPools.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ObjectPool.h"
#include "ObjectWithData.h"
#include <future>
#include <random>
#include <chrono>
#include <thread>

int main() {

    size_t totalObjects = 5;
    const size_t totalTests = 10;

    // Random number generator for sleep and buffer size
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> sleepDis(1, 1000); // Random sleep duration between 1ms and 1000ms
    std::uniform_int_distribution<> bufferDis(1, 1024); // Random buffer size between 1 and 1024

    // Create an Object Pool
    ObjectPool<ObjectWithData> pool(totalObjects);

    // Run the test defined times
    for (auto test = 0; test < totalTests; ++test) {

        std::cout << "Test: " << test << std::endl;

        // Vector to store futures that return the acquired objects
        std::vector<std::future<std::unique_ptr<ObjectWithData>>> futures;

        // RePopulate the Pool Objects Buffers asynchronously
        for (auto i = 0; i < totalObjects; ++i) {

            // Launch each task asynchronously - Emulate multiple thread processing application
            futures.push_back(std::async(std::launch::async, [&pool, i, &sleepDis, &bufferDis, &gen]() {

                // Get the object from the pool
                auto obj = pool.acquire();

                // Randomly allocate buffer size - Test the Object with varying buffer sizes
                auto dataBuffer = obj->allocateBuffer(bufferDis(gen));

                // Random sleep duration - Emulate some processing of the buffer
                std::this_thread::sleep_for(std::chrono::milliseconds(sleepDis(gen)));

                return obj;  // Return the object for tracking
                }));
        }

        // Now we process the futures and release objects back to the pool
        for (auto& future : futures) {

            // Wait for the future to complete and get the acquired object
            auto obj = future.get();

            // Release the object back to the pool after processing is complete
            pool.release(std::move(obj));
        }

        // Increase the total objects by 2 - Grow the pool
        totalObjects += 2;
    }

    return 0;
}

