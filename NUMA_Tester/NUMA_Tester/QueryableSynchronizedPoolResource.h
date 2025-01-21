#ifndef QUERYABLE_SYNCHRONIZED_POOL_RESOURCE_H
#define QUERYABLE_SYNCHRONIZED_POOL_RESOURCE_H

#include <memory_resource>

// Custom synchronized pool resource with query options
class QueryableSynchronizedPoolResource : public std::pmr::synchronized_pool_resource {
public:
    using std::pmr::synchronized_pool_resource::synchronized_pool_resource;

    const std::pmr::pool_options& get_pool_options() const {
        return options();
    }

    std::size_t get_max_blocks_per_chunk() const {
        return options().max_blocks_per_chunk;
    }

    std::size_t get_largest_required_pool_block() const {
        return options().largest_required_pool_block;
    }
};

#endif // QUERYABLE_SYNCHRONIZED_POOL_RESOURCE_H

