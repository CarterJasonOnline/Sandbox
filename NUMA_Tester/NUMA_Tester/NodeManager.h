#ifndef NODE_MANAGER_H
#define NODE_MANAGER_H

#include <vector>
#include <memory>
#include <chrono>
#include "MemoryAllocator.h"
#include "ThreadPool.h"
#include "Constants.h"

// Class to manage the Nodes and the Threads and all the tests
class NodeManager {
public:
    NodeManager(size_t num_nodes, size_t num_threads_per_node, const std::vector<DWORD_PTR>& affinity_masks)
        : num_nodes(num_nodes) {
        // Allocate the MemoryAllocator for each Node
        for (size_t i = 0; i < num_nodes; ++i) {
            std::cout << "Creating MemoryAllocator Node: " << i << std::endl;
            allocators.emplace_back(std::make_unique<MemoryAllocator>(affinity_masks[i]));
        }

        // Allocate the ThreadPool for each Node
        // NOTE: Using SetProcessAffinityMask for the allocs in MemoryAllocator will override the SetThreadAffinityMask for the threads, do them separately
        for (size_t i = 0; i < num_nodes; ++i) {
            std::cout << "Creating ThreadPool Node: " << i << std::endl;
            thread_pools.emplace_back(std::make_unique<ThreadPool>(num_threads_per_node, affinity_masks[i]));
        }
    }

    // Run all the tests
    void run_tests(size_t nLoops, bool withSleep) {

        // Run individually the tests for every node against every node
        // Give BEST and WORST cases for every node
        for (size_t memoryNode = 0; memoryNode < num_nodes; ++memoryNode) {
            for (size_t threadNode = 0; threadNode < num_nodes; ++threadNode) {
                std::cout << "Testing Memory Node: " << memoryNode << " with Thread Node: " << threadNode;
                auto start = std::chrono::high_resolution_clock::now();
                for (size_t nLoop = 0; nLoop < nLoops; ++nLoop) {
                    for (auto frame : allocators[memoryNode]->getFrames()) {
                        thread_pools[threadNode]->enqueue([frame]() {
                            process_frame(frame, FRAME_SIZE);
                            });
                    }
                }
                thread_pools[threadNode]->wait_for_all();
                auto end = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> duration = end - start;
                durations.push_back(duration);
                std::cout << " Duration: " << duration.count() << std::endl;

                if (withSleep) {
                    std::this_thread::sleep_for(std::chrono::seconds(5));
                }
            }
        }

        // Run ALL the tests for every node against every node
        // Give General case when ALL Cores running 
        std::cout << "Testing ALL Nodes GENERAL: ";
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (size_t memoryNode = 0; memoryNode < num_nodes; ++memoryNode) {
                for (size_t threadNode = 0; threadNode < num_nodes; ++threadNode) {

                    for (size_t nLoop = 0; nLoop < nLoops; ++nLoop) {
                        for (auto frame : allocators[memoryNode]->getFrames()) {
                            thread_pools[threadNode]->enqueue([frame]() {
                                process_frame(frame, FRAME_SIZE);
                                });
                        }
                    }
                }
            }

            // Wait for all threads to finish
            for (size_t threadNode = 0; threadNode < num_nodes; ++threadNode)
                thread_pools[threadNode]->wait_for_all();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            durations.push_back(duration);
            std::cout << " Duration: " << duration.count() << std::endl;

            if (withSleep)
                std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        // Run ALL the tests for each node against itself
        // Give BEST case when ALL Cores running
        std::cout << "Testing ALL Nodes BEST: ";
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (size_t memoryNode = 0; memoryNode < num_nodes; ++memoryNode) {
                // Thread and Memory are the same node (BEST case)
                auto threadNode = memoryNode;
                for (size_t nLoop = 0; nLoop < nLoops; ++nLoop) {
                    for (auto frame : allocators[memoryNode]->getFrames()) {
                        thread_pools[threadNode]->enqueue([frame]() {
                            process_frame(frame, FRAME_SIZE);
                            });
                    }
                }
            }

            // Wait for all threads to finish
            for (size_t threadNode = 0; threadNode < num_nodes; ++threadNode)
                thread_pools[threadNode]->wait_for_all();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            durations.push_back(duration);
            std::cout << " Duration: " << duration.count() << std::endl;

            if (withSleep)
                std::this_thread::sleep_for(std::chrono::seconds(5));
        }

        // Run ALL the tests for each node against all the other nodes
        // Give WORST case when ALL Cores running 
        std::cout << "Testing ALL Nodes WORST: ";
        {
            auto start = std::chrono::high_resolution_clock::now();
            for (size_t memoryNode = 0; memoryNode < num_nodes; ++memoryNode) {
                for (size_t threadNode = 0; threadNode < num_nodes; ++threadNode) {
                    // Ignore Self, already tested above as BEST case
                    if (threadNode == memoryNode)
                        continue;

                    for (size_t nLoop = 0; nLoop < nLoops; ++nLoop) {
                        for (auto frame : allocators[memoryNode]->getFrames()) {
                            thread_pools[threadNode]->enqueue([frame]() {
                                process_frame(frame, FRAME_SIZE);
                                });
                        }
                    }
                }
            }

            // Wait for all threads to finish
            for (size_t threadNode = 0; threadNode < num_nodes; ++threadNode)
                thread_pools[threadNode]->wait_for_all();

            auto end = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> duration = end - start;
            durations.push_back(duration);
            std::cout << " Duration: " << duration.count() << std::endl;

            if (withSleep)
                std::this_thread::sleep_for(std::chrono::seconds(5));
        }
    }

    // Print the durations
    void print_durations() const {
        for (const auto& duration : durations) {
            std::cout << "Duration: " << duration.count() << " seconds\n";
        }
    }

private:
    size_t num_nodes;
    std::vector<std::unique_ptr<MemoryAllocator>> allocators;
    std::vector<std::unique_ptr<ThreadPool>> thread_pools;
    std::vector<std::chrono::duration<double>> durations;

    // Helper function to Process a frame
    // This could be easily changed to a more complex function if needed
    static void process_frame(std::byte* frame, size_t size) {
        std::fill(frame, frame + size, std::byte{ 0xAA });
    }
};

#endif // NODE_MANAGER_H

