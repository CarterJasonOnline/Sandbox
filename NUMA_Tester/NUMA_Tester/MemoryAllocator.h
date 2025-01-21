#ifndef MEMORY_ALLOCATOR_H
#define MEMORY_ALLOCATOR_H

#include <vector>
#include <memory>
#include <windows.h>
#include "FixedSizeMemoryResource.h"
#include "QueryableSynchronizedPoolResource.h"
#include "Constants.h"

// Class to wrap Numa Node specific allocations and pools
class MemoryAllocator {
public:
    MemoryAllocator(DWORD_PTR affinityMask) {
        // Save to Restore the Affinity Mask
        hProcess = GetCurrentProcess();
        DWORD_PTR processAffinityMask, systemAffinityMask;
        if (!GetProcessAffinityMask(hProcess, &processAffinityMask, &systemAffinityMask)) {
            std::cerr << "Error getting current affinity mask!" << std::endl;
            return;
        }

        // Use the fact the current affinity of this thread will allocate on the correct NUMA Node
        // NOTE: You could do this with VirtualAllocExNuma as well
        if (!SetProcessAffinityMask(hProcess, affinityMask)) {
            std::cerr << "Error setting affinity mask!" << std::endl;
            return;
        }

        // Allocate a BIG chunk of memory and build up the Pools of Frames
        buffer.resize(POOL_SIZE);
        upstream_resource = std::make_unique<FixedSizeMemoryResource>(buffer.data(), buffer.size());

        options.max_blocks_per_chunk = 128; // Maximum number of blocks per chunk
        options.largest_required_pool_block = FRAME_SIZE; // Largest block size

        pool = std::make_unique<QueryableSynchronizedPoolResource>(options, upstream_resource.get());
        alloc = std::make_unique<ByteAllocator>(pool.get());

        // Allocate as many frames as you can in the Pool
        frames = allocate_max_frames(*alloc);

        // Reset the Affinity Mask
        if (!SetProcessAffinityMask(hProcess, processAffinityMask)) {
            std::cerr << "Error restoring the original affinity mask!" << std::endl;
        }
    }

    // Return a const reference to the vector of frame pointers for use in the Threaded tests
    const std::vector<std::byte*>& getFrames() const {
        return frames;
    }

private:
    HANDLE hProcess;
    std::vector<std::byte> buffer;
    std::unique_ptr<FixedSizeMemoryResource> upstream_resource;
    std::pmr::pool_options options;
    std::unique_ptr<QueryableSynchronizedPoolResource> pool;
    std::unique_ptr<ByteAllocator> alloc;
    std::vector<std::byte*> frames;

    // Helper function to allocate as many frames as possible in the pool
    std::vector<std::byte*> allocate_max_frames(ByteAllocator& alloc) {
        std::vector<std::byte*> frames;
        while (true) {
            try {
                frames.push_back(alloc.allocate(FRAME_SIZE));
            }
            catch (const std::bad_alloc&) {
                break; // Stop allocating when the pool is exhausted
            }
        }
        return frames;
    }
};

#endif // MEMORY_ALLOCATOR_H

