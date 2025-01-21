#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <cstdint>

// Some fixed parameters, these could be Cmd Line Params
constexpr uint64_t FRAME_SIZE = 1920 * 1080 * 3; // Example frame size for 1080p RGB video
constexpr size_t CACHE_LINE_SIZE = 64; // Typical cache line size
constexpr uint64_t POOL_SIZE = 10ULL * 1024ULL * 1024ULL * 1024ULL; // 10GB pool size

using ByteAllocator = std::pmr::polymorphic_allocator<std::byte>;

#endif // CONSTANTS_H