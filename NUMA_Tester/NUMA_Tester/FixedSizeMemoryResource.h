#ifndef FIXED_SIZE_MEMORY_RESOURCE_H
#define FIXED_SIZE_MEMORY_RESOURCE_H

#include <memory_resource>
#include <stdexcept>

class FixedSizeMemoryResource : public std::pmr::memory_resource {
public:
    FixedSizeMemoryResource(void* buffer, size_t size)
        : _buffer(buffer), _size(size), _used(0) {
    }

protected:
    void* do_allocate(size_t bytes, size_t alignment) override {
        static size_t lastbytes = 0;
        if (lastbytes != bytes) {
            lastbytes = bytes;
        }
        void* ptr = static_cast<char*>(_buffer) + _used;
        size_t space = _size - _used;
        void* aligned_ptr = std::align(alignment, bytes, ptr, space);
        if (!aligned_ptr) {
            throw std::bad_alloc();
        }
        _used = _size - space + bytes;
        return aligned_ptr;
    }

    void do_deallocate(void*, size_t, size_t) override {
        // No deallocation for monotonic buffer
    }

    bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
        return this == &other;
    }

private:
    void* _buffer;
    size_t _size;
    size_t _used;
};

#endif // FIXED_SIZE_MEMORY_RESOURCE_H
